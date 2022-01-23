/*
BSD 2-Clause License

Copyright (c) 2020, Till Mayer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "Dtmf.hpp"
#include <FL/Fl_Button.H>
#include <dsp/GoertzelFilter.hpp>
#include <dsp/Mapper.hpp>
#include <pipe/Parallel.hpp>
#include <util/Types.hpp>

using namespace Dsp;

static constexpr SampleRate sample_rate { 4000 };
static constexpr Taps filter_taps { sample_rate / 50 };                                           //About 50 Hz per bin
static constexpr Samples required_samples_per_symbol { static_cast<Samples>(sample_rate * .05) }; //> 50 ms
static constexpr Samples minimum_samples_per_space { static_cast<Samples>(sample_rate * .01) };   //> 10 ms
static constexpr Samples minimum_samples_per_block { static_cast<Samples>(sample_rate * .5) };    //> 500 ms
static constexpr std::array map { '1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D' };

Dtmf::Dtmf()
    : Decoder<u8>("DTMF", sample_rate, DecoderBase::Headless::Yes, 130) {
}

u8 find_max(const Buffer<float>& inputs) {
    float max = inputs[0];
    u8 max_index = 0;

    for (u8 i = 0; i < inputs.size(); ++i) {
        const float value = inputs[i];
        if (value > max) {
            max_index = i;
            max = value;
        }
    }

    if (max == 0)
        Pipe::GenericComponent::abort_processing();
    return max_index;
}

static Pipe::Parallel<float, float, u8> row_filter_bank() {
    std::function<u8(const Buffer<float>&)> func = find_max;
    return Pipe::parallel(func,
                          GoertzelFilter(filter_taps, 1209),
                          GoertzelFilter(filter_taps, 1336),
                          GoertzelFilter(filter_taps, 1477),
                          GoertzelFilter(filter_taps, 1633));
}

static Pipe::Parallel<float, float, u8> column_filter_bank() {
    std::function<u8(const Buffer<float>&)> func = find_max;
    return Pipe::parallel(func,
                          GoertzelFilter(filter_taps, 697),
                          GoertzelFilter(filter_taps, 770),
                          GoertzelFilter(filter_taps, 852),
                          GoertzelFilter(filter_taps, 941));
}

Fl_Widget* Dtmf::build_ui(Point top_left, Size ui_size) {
    m_callback_manager.forget_callbacks();
    auto* root = new Fl_Group(top_left.x(), top_left.y(), ui_size.w(), ui_size.h());

    auto* controls = new Fl_Group(top_left.x(), top_left.y(), ui_size.w(), 44);
    controls->box(FL_EMBOSSED_BOX);

    auto control_offset = top_left.translated(4, 4);
    Size control_size(ui_size.w() - 8, controls->h() - 8);

    m_detect_indicator = new Ui::Indicator(control_offset.x() + 10,
                                           control_offset.y(),
                                           40,
                                           control_size.h(),
                                           Ui::Indicator::green_on,
                                           Ui::Indicator::green_off,
                                           "Detected");

    auto* clear_button = new Fl_Button(controls->x() + controls->w() - 54, controls->y() + 4, 50, controls->h() - 8, "Clear");
    m_callback_manager.register_callback(*clear_button, [&]() { m_text_box->clear(); });

    auto* spring = new Fl_Box(clear_button->x(), clear_button->y(), 0, 0);
    controls->resizable(spring);
    controls->end();

    top_left.translate(0, controls->h() + 2);
    ui_size.resize(0, -controls->h() - 2);

    m_text_box = new Ui::TextDisplay(top_left.x(), top_left.y(), ui_size.w(), ui_size.h());
    m_text_box->textfont(FL_COURIER);
    root->resizable(m_text_box);
    root->end();
    return root;
}

void Dtmf::process_pipeline_result(u8 sample) {
    const u8 row = sample % 10;
    const u8 column = sample / 10;
    const char received = map[column + row * 4];

    if (received != m_last_symbol) {
        if (m_sample_count > required_samples_per_symbol) {
            if (m_last_valid == m_last_symbol && (m_last_interruption_length < minimum_samples_per_space && m_last_interruption_length > 0)) {
                m_last_interruption_length = 0;
                return;
            }

            const char decoded = m_last_symbol;
            m_last_valid = m_last_symbol;
            m_last_interruption_length = 0;
            m_last_symbol = received;
            m_sample_count = 1;
            m_samples_since_last_valid_symbol = 1;

            if (Drtd::using_ui()) {
                const char buf[] { decoded, 0 };
                m_text_box->buffer()->append(buf);
                m_detect_indicator->set_state(true);
            } else {
                printf("%c", decoded);
                fflush(stdout);
            }
        } else {
            m_last_interruption_length += m_sample_count;
        }

        m_last_symbol = received;
        m_sample_count = 1;
    } else {
        ++m_sample_count;
    }

    if (m_samples_since_last_valid_symbol > minimum_samples_per_block && m_samples_since_last_valid_symbol) {
        if (Drtd::using_ui()) {
            const bool scroll = m_text_box->should_autoscroll();
            m_text_box->buffer()->append("\n");
            if (scroll)
                m_text_box->scroll_to_bottom();
            m_detect_indicator->set_state(false);
        } else {
            printf("\n");
        }

        m_samples_since_last_valid_symbol = 0;
    } else if (m_samples_since_last_valid_symbol) {
        ++m_samples_since_last_valid_symbol;
    }
}

Pipe::Line<float, u8> Dtmf::build_pipeline() {
    std::function<u8(const Buffer<u8>&)> func = [](const Buffer<u8>& in) { return in[0] + in[1] * 10; };
    return Pipe::line(Pipe::parallel(func, column_filter_bank(), row_filter_bank()));
}

