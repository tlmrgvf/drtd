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
#include "Pocsag.hpp"
#include "PocsagMessage.hpp"
#include <dsp/BitConverter.hpp>
#include <dsp/FirFilter.hpp>
#include <dsp/Mapper.hpp>
#include <dsp/MovingAverage.hpp>
#include <util/Config.hpp>

using namespace Dsp;

static constexpr SampleRate sample_rate { 12000 };
static constexpr Samples bits_required_for_sync { 25 };

Pocsag::Pocsag()
    : Decoder<bool>("POCSAG", sample_rate, DecoderBase::Headless::Yes, 140) {
}

std::string Pocsag::state_string(State state) {
    switch (state) {
    case State::FirstBitSinceSync:
        return "Waiting for preamble...";
    case State::WaitForInitialSyncWord:
        return "Waiting for sync...";
    case State::WaitForImmediateSyncWord:
        return "Waiting for immediate sync...";
    case State::ReadPocsagBatch:
        return "Reading batch...";
    }

    assert(false);
    return "Invalid";
}

void Pocsag::update_state(State state) {
    m_state = state;
    set_status(state_string(state));
}

Fl_Widget* Pocsag::build_ui(Point top_left, Size ui_size) {
    m_callback_manager.forget_callbacks();
    auto* root = new Fl_Group(top_left.x(), top_left.y(), ui_size.w(), ui_size.h());

    auto* controls = new Fl_Group(top_left.x(), top_left.y(), ui_size.w(), 44);
    controls->box(FL_EMBOSSED_BOX);
    auto control_offset = top_left.translated(4, 4);
    Size control_size(ui_size.w() - 8, controls->h() - 8);

    m_sync_512_indicator = new Ui::Indicator(control_offset.x(),
                                             control_offset.y(),
                                             40,
                                             control_size.h(),
                                             Ui::Indicator::yellow_on,
                                             Ui::Indicator::yellow_off,
                                             "512");
    m_sync_512_indicator->set_state(false);
    control_offset.translate(m_sync_512_indicator->w() + 2, 0);

    m_sync_1200_indicator = new Ui::Indicator(control_offset.x(),
                                              control_offset.y(),
                                              40,
                                              control_size.h(),
                                              Ui::Indicator::yellow_on,
                                              Ui::Indicator::yellow_off,
                                              "1200");
    m_sync_1200_indicator->set_state(false);
    control_offset.translate(m_sync_1200_indicator->w() + 2, 0);

    m_sync_2400_indicator = new Ui::Indicator(control_offset.x(),
                                              control_offset.y(),
                                              40,
                                              control_size.h(),
                                              Ui::Indicator::yellow_on,
                                              Ui::Indicator::yellow_off,
                                              "2400");
    m_sync_2400_indicator->set_state(false);
    control_offset.translate(m_sync_2400_indicator->w() + 2, 0);

    m_data_indicator = new Ui::Indicator(control_offset.x(),
                                         control_offset.y(),
                                         40,
                                         control_size.h(),
                                         Ui::Indicator::green_on,
                                         Ui::Indicator::green_off,
                                         "Data");

    m_content_selector = new Fl_Choice(top_left.x() + 4 + control_size.w() - 185, control_offset.y(), 135, control_size.h(), "Show: ");
    for (u8 i = 0; i < static_cast<u8>(PocsagProtocol::Message::ContentType::__Count); ++i)
        m_content_selector->add(PocsagProtocol::Message::content_name(static_cast<PocsagProtocol::Message::ContentType>(i)).c_str());
    m_content_selector->value(static_cast<int>(m_content_type));

    m_callback_manager.register_callback(*m_content_selector, [&]() {
        m_content_type = static_cast<PocsagProtocol::Message::ContentType>(m_content_selector->value());
    });

    auto* clear_button = new Fl_Button(m_content_selector->x() + m_content_selector->w() + 2, control_offset.y(), 50, control_size.h(), "Clear");
    m_callback_manager.register_callback(*clear_button, [&]() { m_text_box->clear(); });

    auto* spring = new Fl_Box(m_content_selector->x(), m_content_selector->y(), 0, 0);
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

void Pocsag::on_setup() {
    reset(false);
    if (Drtd::using_ui())
        Util::Config::load(config_path() + ".ContentType", m_content_type, PocsagProtocol::Message::ContentType::AlphaNumeric);
}

void Pocsag::on_tear_down() {
    if (Drtd::using_ui())
        Util::Config::save(config_path() + ".ContentType", m_content_type);
}

void Pocsag::reset(bool reset_indicators) {
    if (m_matched_filter.valid())
        m_matched_filter->set_taps(1);

    if (m_converter.valid())
        m_converter->wait_for_sync();

    m_codeword_count = 0;
    m_inverted = false;
    m_incoming_buffer.reset();
    m_preamble_count = 0;
    update_state(State::FirstBitSinceSync);
    m_message_builder = {};

    if (reset_indicators && Drtd::using_ui()) {
        m_sync_512_indicator->set_state(false);
        m_sync_1200_indicator->set_state(false);
        m_sync_2400_indicator->set_state(false);
        m_data_indicator->set_state(false);
    }
}

Pipe::Line<float, bool> Pocsag::build_pipeline() {
    auto moving_average = MovingAverage<float>(1);
    auto converter = BitConverter(bits_required_for_sync, { 512, 1200, 2400 });

    m_matched_filter = moving_average.make_ref();
    m_converter = converter.make_ref();

    converter.sync_callback = [&](BitConverter::SyncInfo sync_info) {
        m_matched_filter->set_taps(static_cast<Taps>(std::roundf(sync_info.samples_per_bit)));
    };

    return Pipe::line(std::move(moving_average),
                      Mapper<float, bool>([](auto input) { return input < 0; }),
                      std::move(converter));
}

void Pocsag::message_done() {
    const auto message = m_message_builder.build(m_content_type, static_cast<BaudRate>(m_converter->get_current_baud_rate()));
    if (m_message_builder.valid()) {
        if (Drtd::using_ui()) {
            const bool scroll = m_text_box->should_autoscroll();
            m_text_box->buffer()->append(message.str().c_str());
            if (scroll)
                m_text_box->scroll_to_bottom();
        } else {
            puts(message.str().c_str());
        }
    }

    m_message_builder = {};
}

Buffer<std::string> Pocsag::changeable_parameters() const {
    return { "None/Alpha/Numeric/Both" };
}

bool Pocsag::setup_parameters(const Buffer<std::string>& args) {
    const auto& arg = Util::to_lower(args[0]);

    if (arg == "none")
        m_content_type = PocsagProtocol::Message::ContentType::NoContent;
    else if (arg == "alpha")
        m_content_type = PocsagProtocol::Message::ContentType::AlphaNumeric;
    else if (arg == "numeric")
        m_content_type = PocsagProtocol::Message::ContentType::Numeric;
    else if (arg == "both")
        m_content_type = PocsagProtocol::Message::ContentType::Both;
    else
        return false;

    return true;
}

void Pocsag::process_pipeline_result(bool sample) {
    const bool update_ui = Drtd::using_ui();
    sample ^= m_inverted;
    m_received_parity ^= sample;
    m_incoming_buffer.push(sample);
    if (update_ui)
        m_data_indicator->set_state(sample);

    std::optional<u32> code_word;
    switch (m_state) {
    case State::FirstBitSinceSync:
        m_last_bit = sample;
        update_state(State::WaitForInitialSyncWord);
        break;
    case State::WaitForInitialSyncWord:
        m_incoming_buffer.reset_bit_count();
        if (sample == m_last_bit && m_preamble_count < PocsagProtocol::Data::preamble_bit_count / 4) {
            logger().info() << "Invalid preamble";
            reset(true);
            return;
        }

        m_last_bit = sample;
        if (++m_preamble_count > PocsagProtocol::Data::preamble_bit_count * 3) {
            logger().info() << "Preamble too long!";
            reset(true);
            return;
        }
        [[fallthrough]];
    case State::WaitForImmediateSyncWord:
        if (m_state != State::WaitForInitialSyncWord && !m_incoming_buffer.aligned())
            return;

        code_word = m_incoming_buffer.data<u32>();
        if (m_state == State::WaitForImmediateSyncWord) {
            code_word = m_bch_code.correct((code_word.value() >> 1) & ~0x80000000);
            if (code_word.has_value())
                code_word = code_word.value() << 1;
        }

        if (code_word.has_value()) {
            if (code_word.value() == PocsagProtocol::Data::sync_word) {
                logger().info() << "Sync...";
            } else if (code_word.value() == ~PocsagProtocol::Data::sync_word) {
                logger().info() << "Inverted Sync detected, inverting all other bits from here on out!";
                if (m_state == State::WaitForInitialSyncWord)
                    m_inverted = true;
            } else {
                break;
            }
        } else {
            logger().info() << "Did not get expected sync codeword, message done.";
            message_done();
            reset(true);
            break;
        }

        update_state(State::ReadPocsagBatch);
        if (update_ui) {
            if (m_converter->get_current_baud_rate() == 512)
                m_sync_512_indicator->set_state(true);
            else if (m_converter->get_current_baud_rate() == 1200)
                m_sync_1200_indicator->set_state(true);
            else if (m_converter->get_current_baud_rate() == 2400)
                m_sync_2400_indicator->set_state(true);
        }
        m_received_parity = true;
        break;
    case State::ReadPocsagBatch:
        if (!m_incoming_buffer.aligned())
            return;

        /* POCSAG uses even parity */
        if (!m_received_parity)
            logger().info() << "Parity error!";

        code_word = m_bch_code.correct((m_incoming_buffer.data<u32>() >> 1) & ~0x80000000);
        if (code_word.has_value()) {
            const auto data = PocsagProtocol::Data::from_codeword(m_codeword_count, code_word.value());
            m_message_builder.append_data(data);
            if (data.type() == PocsagProtocol::Data::Type::Idle || data.type() == PocsagProtocol::Data::Type::Address)
                message_done();
        } else {
            logger().info() << "Could not correct code word!";
            m_message_builder.set_has_invalid_codeword();
        }

        if (++m_codeword_count >= PocsagProtocol::Message::codewords_per_batch) {
            update_state(State::WaitForImmediateSyncWord);
            m_codeword_count = 0;
        }

        m_received_parity = true;
        break;
    default:
        assert(false);
    }
}
