#include "Ax25.hpp"
#include "Packet.hpp"
#include <Fl/Fl_Slider.H>
#include <dsp/AngleDifference.hpp>
#include <dsp/BiquadFilterComponent.hpp>
#include <dsp/BitConverter.hpp>
#include <dsp/FirFilter.hpp>
#include <dsp/IQMixer.hpp>
#include <dsp/Mapper.hpp>
#include <dsp/MovingAverage.hpp>
#include <dsp/NRZIDecoder.hpp>
#include <ui/component/Indicator.hpp>
#include <util/Cmplx.hpp>

using namespace Dsp;

static constexpr u16 sample_rate { 22050 };
static constexpr u16 baud_rate { 1200 };

Ax25::Ax25()
    : Decoder<bool>("AX.25", sample_rate, DecoderBase::Headless::Yes, 140) {
    set_marker({ { { -500, 100 }, { 500, 100 } }, false });
    set_center_frequency(1700);
}

Ax25::StateInfo Ax25::info_for_state(State state) {
    switch (state) {
    case State::WaitFlag:
        return { .ignore_stuffed_bits = false,
                 .update_indicator = false,
                 .label = "Waiting for flag..." };
    case State::CountFlag:
        return { .ignore_stuffed_bits = false,
                 .update_indicator = false,
                 .label = "Counting flags..." };
    case State::WaitData:
        return { .ignore_stuffed_bits = false,
                 .update_indicator = true,
                 .label = "Waiting for begin of data..." };
    case State::WaitEnd:
        return { .ignore_stuffed_bits = true,
                 .update_indicator = true,
                 .label = "Reading data..." };
    }

    assert(false);
    return {};
}

void Ax25::on_setup() {
    change_state_to(State::WaitFlag);
}

void Ax25::change_state_to(State new_state) {
    m_current_state_info = info_for_state(new_state);
    if (m_current_state_info.ignore_stuffed_bits) {
        m_delay_buffer.reset();
        m_processed_buffer.reset();
        m_one_count = 0;
    }

    m_in_buffer.reset_bit_count();
    m_state = new_state;
    set_status(m_current_state_info.label);
}

Fl_Widget* Ax25::build_ui(Point top_left, Size ui_size) {
    m_callback_manager.forget_callbacks();
    auto* root = new Fl_Group(top_left.x(), top_left.y(), ui_size.w(), ui_size.h());

    auto* controls = new Fl_Group(top_left.x(), top_left.y(), ui_size.w(), 44);
    controls->box(FL_EMBOSSED_BOX);
    auto control_offset = top_left.translated(4, 4);
    Size control_size(ui_size.w() - 8, controls->h() - 8);

    m_sync_indicator = new Ui::Indicator(control_offset.x(), control_offset.y(), 40, control_size.h(), Ui::Indicator::yellow_on, Ui::Indicator::yellow_off, "Sync");
    m_sync_indicator->set_state(false);

    m_data_indicator = new Ui::Indicator(m_sync_indicator->x() + m_sync_indicator->w() + 2,
                                         control_offset.y(),
                                         40,
                                         control_size.h(),
                                         Ui::Indicator::green_on,
                                         Ui::Indicator::green_off,
                                         "Data");

    auto* clear_button = new Fl_Button(control_offset.x() + control_size.w() - 50, control_offset.y(), 50, control_size.h(), "Clear");
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

Pipe::Line<float, bool> Ax25::build_pipeline() {
    return Pipe::line(
        IQMixer(1700),
        //        Biquad::FilterComponent<Cmplx>(Biquad::Type::Lowpass, 1200, Biquad::invsqrt2),
        FirFilter<Cmplx>(WindowType::Hamming, 41, 0, 600),
        AngleDifference(),
        MovingAverage<float>(std::round(sample_rate / static_cast<float>(baud_rate))),
        Mapper<float, bool>([](float sample) { return sample < 0; }),
        BitConverter(baud_rate),
        NRZIDecoder(true));
}

void Ax25::packet_done() {
    if (Drtd::using_ui()) {
        m_data_indicator->set_state(false);
        m_sync_indicator->set_state(false);
    }

    auto packet = AX25Protocol::Packet::parse(m_packet_buffer);
    m_packet_buffer.clear();

    if (!packet.has_value())
        return;

    if (Drtd::using_ui()) {
        const bool autoscroll = m_text_box->should_autoscroll();
        m_text_box->buffer()->append(packet->format().c_str());
        if (autoscroll)
            m_text_box->scroll_to_bottom();
    } else {
        puts(packet->format().c_str());
    }
}

void Ax25::process_pipeline_result(bool bit) {
    bool in_bit = m_delay_buffer.push(m_in_buffer.push(bit));

    if (m_one_count >= 5) {
        m_one_count = 0;
        if (in_bit && m_state == State::WaitEnd) {
            logger().warning() << "Expected zero is one, assuming packet is done?";
            packet_done();
            return;
        }
    } else {

        if (in_bit)
            ++m_one_count;
        else
            m_one_count = 0;
        m_processed_buffer.push(in_bit);
    }

    auto in_byte = m_in_buffer.data<u8>();
    if (m_current_state_info.update_indicator && m_data_indicator)
        m_data_indicator->set_state(bit);

    switch (m_state) {
    case State::WaitFlag:
        if (in_byte == AX25Protocol::Packet::magic_flag) {
            change_state_to(State::CountFlag);
            m_header_count = 1;
        }

        break;
    case State::CountFlag:
        if (!m_in_buffer.aligned())
            return;

        if (in_byte == AX25Protocol::Packet::magic_flag) {
            if (++m_header_count >= headers_needed)
                change_state_to(State::WaitData);
        } else {
            m_header_count = 0;
            change_state_to(State::WaitFlag);
        }

        break;
    case State::WaitData:
        if (m_sync_indicator)
            m_sync_indicator->set_state(true);
        if (m_in_buffer.aligned() && in_byte != AX25Protocol::Packet::magic_flag)
            change_state_to(State::WaitEnd);

        break;
    case State::WaitEnd:
        if (in_byte == AX25Protocol::Packet::magic_flag) {
            change_state_to(State::WaitFlag);
            packet_done();
            return;
        } else if (m_processed_buffer.aligned()) {
            m_packet_buffer.push_back(m_processed_buffer.data<u8>());
        }
        break;
    }
}
