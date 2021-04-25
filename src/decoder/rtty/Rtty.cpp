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
#include "Rtty.hpp"
#include <dsp/Mapper.hpp>
#include <pipe/Parallel.hpp>
#include <util/Config.hpp>
#include <util/Types.hpp>

using namespace Dsp;

struct BaudotCode {
    constexpr BaudotCode() = default;

    constexpr BaudotCode(const char* letter_str, const char* figure_str)
        : letter(letter_str)
        , figure(figure_str) {}

    const char* letter { nullptr };
    const char* figure { nullptr };
};

static constexpr u8 figures { 0x1B };
static constexpr u8 letters { 0x1F };
static constexpr std::array baudot_codes {
    BaudotCode("", ""),
    BaudotCode("E", "3"),
    BaudotCode("\n", "\n"),
    BaudotCode("A", "-"),
    BaudotCode(" ", " "),
    BaudotCode("S", "<BEL>"),
    BaudotCode("I", "8"),
    BaudotCode("U", "7"),
    BaudotCode("\r", "\r"),
    BaudotCode("D", "$"),
    BaudotCode("R", "4"),
    BaudotCode("J", "'"),
    BaudotCode("N", ","),
    BaudotCode("F", "!"),
    BaudotCode("C", ":"),
    BaudotCode("K", "("),
    BaudotCode("T", "5"),
    BaudotCode("Z", "\""),
    BaudotCode("L", ")"),
    BaudotCode("W", "2"),
    BaudotCode("H", "#"),
    BaudotCode("Y", "6"),
    BaudotCode("P", "0"),
    BaudotCode("Q", "1"),
    BaudotCode("O", "9"),
    BaudotCode("B", "?"),
    BaudotCode("G", "&"),
    BaudotCode(), //FIGURES is 0x1B
    BaudotCode("M", "."),
    BaudotCode("X", "/"),
    BaudotCode("V", ";"),
    BaudotCode() //LETTERS is 0x1F
};

Rtty::Rtty()
    : Decoder<bool>("RTTY", sample_rate, DecoderBase::Headless::Yes, 160) {
}

void Rtty::update_marker() {
    Util::MarkerGroup group;
    group.moveable = true;
    group.markers = { { .offset = -static_cast<i32>(m_settings.shift) / 2, .bandwidth = static_cast<Hertz>(m_settings.baud_rate) },
                      { .offset = m_settings.shift / 2, .bandwidth = static_cast<Hertz>(m_settings.baud_rate) } };
    set_marker(group);
}

void Rtty::on_setup() {
    if (Drtd::using_ui())
        Util::Config::load(config_path() + ".Settings", m_settings, {});

    update_marker();
    update_filters();
    update_mixers();
}

void Rtty::on_tear_down() {
    if (Drtd::using_ui())
        Util::Config::save(config_path() + ".Settings", m_settings);
}

void Rtty::update_filters() {
    m_converter->set_baud_rates({ m_settings.baud_rate });
    const Samples samples_per_bit = static_cast<Samples>(sample_rate / m_settings.baud_rate);

    m_mark_filter->set_taps(samples_per_bit);
    m_space_filter->set_taps(samples_per_bit);

    /* At least 6 bits can be zero */
    m_mark_normalizer->set_window_size(samples_per_bit * 7);
    m_space_normalizer->set_window_size(samples_per_bit * 7);
}

void Rtty::update_mixers() {
    m_space_mixer->set_frequency(center_frequency() - m_settings.shift / 2);
    m_mark_mixer->set_frequency(center_frequency() + m_settings.shift / 2);
}

void Rtty::update_scope(float mark, float space) {
    if (Drtd::using_ui() && m_scope->visible()) {
        m_scope_phase = std::remainder(m_scope_phase + scope_phase_step, Util::two_pi_f);
        m_scope->process(Cmplx(mark * cosf(m_scope_phase), space * sinf(m_scope_phase)));
    }
}

void Rtty::on_marker_move(Hertz) {
    update_mixers();
}

Fl_Widget* Rtty::build_ui(Util::Point top_left, Util::Size size) {
    m_callback_manager.forget_callbacks();

    auto* root = new Fl_Group(top_left.x(), top_left.y(), size.w(), size.h());
    auto* controls = new Fl_Group(top_left.x(), top_left.y(), size.w(), 70);
    controls->box(FL_EMBOSSED_BOX);
    auto control_offset = top_left.translated(4, 4);
    Size control_size(size.w() - 8, controls->h() - 8);

    auto* shift = new Fl_Spinner(control_offset.x() + 75, control_offset.y(), 100, 30, "Shift:");
    shift->value(m_settings.shift);
    shift->step(1);
    shift->range(0, 2000);
    m_callback_manager.register_callback(*shift, [&, shift]() {
        m_settings.shift = static_cast<float>(shift->value());
        update_mixers();
        update_marker();
    });

    auto* baud_rate = new Fl_Spinner(shift->x(), shift->y() + shift->h() + 2, shift->w(), 30, "Baudrate:");
    baud_rate->value(m_settings.baud_rate);
    baud_rate->step(.01);
    baud_rate->range(10, 300);
    m_callback_manager.register_callback(*baud_rate, [&, baud_rate]() {
        m_settings.baud_rate = static_cast<float>(baud_rate->value());
        update_marker();
        update_filters();
    });

    auto* mark_space_swap = new Fl_Check_Button(shift->x() + shift->w() + 6, shift->y(), 175, 30, "Swap mark and space");
    mark_space_swap->value(m_settings.swap_mark_and_space);
    m_callback_manager.register_callback(*mark_space_swap, [&, mark_space_swap]() {
        m_settings.swap_mark_and_space = static_cast<bool>(mark_space_swap->value());
    });

    auto* show_scope = new Fl_Check_Button(mark_space_swap->x(), mark_space_swap->y() + mark_space_swap->h() + 2, 170, 30, "Show tuning scope");
    m_callback_manager.register_callback(*show_scope, [&, show_scope]() {
        if (show_scope->value())
            m_scope->show();
        else
            m_scope->hide();
        m_settings.show_tuning = static_cast<bool>(show_scope->value());
    });
    show_scope->value(m_settings.show_tuning);

    auto* clear_button = new Fl_Button(control_offset.x() + control_size.w() - 60, control_offset.y() + Util::center(control_size.h(), 30), 60, 30, "Clear");
    m_callback_manager.register_callback(*clear_button, [&]() {
        m_text_box->clear();
    });

    m_scope = new Ui::XYScope(mark_space_swap->x() + mark_space_swap->w() + 4, control_offset.y() + Util::center(control_size.h(), 60), 60, 200, .90f, true);
    if (!m_settings.show_tuning)
        m_scope->hide();

    auto* spring = new Fl_Box(clear_button->x(), clear_button->y(), 0, 0);
    controls->resizable(spring);
    controls->end();

    top_left.translate(0, controls->h() + 2);
    size.resize(0, -controls->h() - 2);

    m_text_box = new Ui::TextDisplay(top_left.x(), top_left.y(), size.w(), size.h());
    root->resizable(m_text_box);
    root->end();
    return root;
}

void Rtty::process_pipeline_result(bool sample) {
    /*
     *   RTTY is "idle on mark"
     *   Start bit  5 Baudot bits  1, 1.5 or 2 stop bits
     *   0          XXXXX          1(1)
     */

    sample = sample ^ m_settings.swap_mark_and_space;
    m_input_buffer.push(sample);

    if (m_wait_start && !m_input_buffer.get(0) && m_input_buffer.get(6)) {
        m_wait_start = false;
        m_input_buffer.reset_bit_count();
    } else if (m_wait_start || !m_input_buffer.aligned()) {
        return;
    }

    if (m_input_buffer.get(0) || !m_input_buffer.get(6)) {
        m_wait_start = true;
        return;
    }

    u8 bits = (m_input_buffer.data<u8>() >> 1) & 0x1F;
    if (bits == letters) {
        m_figures = false;
    } else if (bits == figures) {
        m_figures = true;
    } else {
        BaudotCode code = baudot_codes[bits];
        const char* to_add = m_figures ? code.figure : code.letter;

        if (Drtd::using_ui()) {
            bool autoscroll = m_text_box->should_autoscroll();
            m_text_box->buffer()->append(to_add);
            if (autoscroll)
                m_text_box->scroll_to_bottom();
        } else {
            printf("%s", to_add);
            std::fflush(stdout);
        }
    }
}

Pipe::Line<float, bool> Rtty::build_pipeline() {
    IQMixer mark_mixer(0);
    IQMixer space_mixer(0);
    MovingAverage<Cmplx> mark_filter(1);
    MovingAverage<Cmplx> space_filter(1);
    Normalizer mark_normalizer(1, Normalizer::Lookahead::Yes, Normalizer::OffsetMode::Minimum);
    Normalizer space_normalizer(1, Normalizer::Lookahead::Yes, Normalizer::OffsetMode::Minimum);
    BitConverter converter(1, { m_settings.baud_rate });

    m_mark_mixer = mark_mixer.make_ref();
    m_space_mixer = space_mixer.make_ref();
    m_mark_filter = mark_filter.make_ref();
    m_space_filter = space_filter.make_ref();
    m_mark_normalizer = mark_normalizer.make_ref();
    m_space_normalizer = space_normalizer.make_ref();
    m_converter = converter.make_ref();

    auto mark_detector = Pipe::line(
        std::move(mark_mixer),
        std::move(mark_filter),
        Mapper<Cmplx, float>([](Cmplx in) { return in.magnitude_squared(); }),
        std::move(mark_normalizer));

    auto space_detector = Pipe::line(
        std::move(space_mixer),
        std::move(space_filter),
        Mapper<Cmplx, float>([](Cmplx in) { return in.magnitude_squared(); }),
        std::move(space_normalizer));

    std::function<bool(const Buffer<float>&)> compare_space_mark = [&](const auto& results) {
        update_scope(results[0], results[1]);
        return (results[0] - results[1]) > 0;
    };

    return Pipe::line(Pipe::parallel(compare_space_mark, std::move(mark_detector), std::move(space_detector)), std::move(converter));
}

Util::Buffer<std::string> Rtty::changeable_parameters() const {
    return { "Center Frequency (Integer)", "Shift (Integer)", "Baud rate (Float)", "USB/LSB" };
}

bool Rtty::setup_parameters(const Util::Buffer<std::string>& parameters) {
    assert(parameters.size() == 4);
    int center_frequency = Util::parse_int(parameters[0]).value_or(-1);
    if (center_frequency < 0) {
        puts("Invalid center frequency!");
        return false;
    }

    int shift = Util::parse_int(parameters[1]).value_or(-1);
    if (shift < 0) {
        puts("Invalid shift!");
        return false;
    }

    float baud_rate = Util::parse_float(parameters[2]).value_or(-1);
    if (baud_rate < 10) {
        puts("Invalid baud rate!");
        return false;
    }

    if (Util::to_lower(parameters[3]) != "usb" && Util::to_lower(parameters[3]) != "lsb") {
        puts("Provide either USB or LSB");
        return false;
    }

    set_center_frequency(center_frequency);
    m_settings.baud_rate = baud_rate;
    m_settings.shift = shift;
    m_settings.swap_mark_and_space = parameters[3] == "LSB";
    return true;
}
