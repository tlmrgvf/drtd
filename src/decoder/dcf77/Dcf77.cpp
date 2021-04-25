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
#include "Dcf77.hpp"
#include <dsp/Mapper.hpp>
#include <dsp/MovingAverage.hpp>
#include <dsp/Normalizer.hpp>
#include <iomanip>
#include <util/Util.hpp>

using namespace Dsp;

static constexpr SampleRate sample_rate { 6000 };
static constexpr BaudRate baud_rate { 10 };
static constexpr Samples samples_per_bit { sample_rate / baud_rate };
static constexpr u8 status_mask_call { 0b10000 };
static constexpr u8 status_mask_cest { 0b100 };
static constexpr u8 status_mask_cet { 0b10 };
static constexpr u8 status_mask_leap_second { 0b1 };
static constexpr std::array bcd_lookup { 1, 2, 4, 8, 10, 20, 40, 80 };
static constexpr std::array dow_lookup { "---", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

Dcf77::Dcf77()
    : Decoder<bool>("DCF77", sample_rate, DecoderBase::Headless::Yes, 240) {
    set_marker({ .markers = { Util::Marker { .offset = 0, .bandwidth = 10 } }, .moveable = true });
}

static u8 decode_bcd(u16 bits, u8 bit_count) {
    u8 result = 0;
    for (u8 i = 0; i < bit_count; ++i) {
        if ((bits & 1) != 0)
            result += bcd_lookup[bit_count - i - 1];
        bits >>= 1;
    }

    return result;
}

void Dcf77::on_setup() {
    m_state = State::WaitForMinuteMarker;
    m_bits_received = 0;
    m_time = {};
    m_receiving = {};
    m_bits = 0;
    set_status(info_for_state(m_state).description);
}

Fl_Widget* Dcf77::build_ui(Point top_left, Size ui_size) {
    auto* root = new Fl_Group(top_left.x(), top_left.y(), ui_size.w(), ui_size.h());
    m_date_label = new Fl_Box(top_left.x(), top_left.y(), ui_size.w(), 40, "---, --.--.----");
    m_date_label->labelsize(35);
    m_date_label->labelfont(FL_BOLD);

    m_time_label = new Fl_Box(top_left.x(), m_date_label->y() + m_date_label->h() + 4, ui_size.w(), 120, "--:--:--");
    m_time_label->labelsize(100);
    m_time_label->labelfont(FL_BOLD);

    u16 indicator_offset_y = m_time_label->y() + m_time_label->h();
    auto* indicators = new Fl_Group(top_left.x(), indicator_offset_y, ui_size.w(), ui_size.h() - (indicator_offset_y - top_left.y()));
    Point indicator_pos(top_left.x() + 4, indicator_offset_y + 4);
    u16 indicator_height = indicators->h() - 8;
    constexpr u8 indicator_width = 72;
    constexpr u8 indicator_spacing = 4;

    m_rx_indicator = new Ui::Indicator(indicator_pos.x(),
                                       indicator_pos.y(),
                                       indicator_width,
                                       indicator_height,
                                       Ui::Indicator::green_on,
                                       Ui::Indicator::green_off,
                                       "Receiving");

    indicator_pos.translate(indicator_width + indicator_spacing, 0);
    m_rx_fail_indicator = new Ui::Indicator(indicator_pos.x(),
                                            indicator_pos.y(),
                                            indicator_width,
                                            indicator_height,
                                            Ui::Indicator::red_on,
                                            Ui::Indicator::red_off,
                                            "\nReceive\nfailed");

    indicator_pos.translate(indicator_width + indicator_spacing, 0);
    m_cet_indicator = new Ui::Indicator(indicator_pos.x(),
                                        indicator_pos.y(),
                                        indicator_width,
                                        indicator_height,
                                        Ui::Indicator::green_on,
                                        Ui::Indicator::green_off,
                                        "\nCET\n(+1 UTC)");

    indicator_pos.translate(indicator_width + indicator_spacing, 0);
    m_cest_indicator = new Ui::Indicator(indicator_pos.x(),
                                         indicator_pos.y(),
                                         indicator_width, indicator_height,
                                         Ui::Indicator::green_on,
                                         Ui::Indicator::green_off,
                                         "\nCEST\n(+2 UTC)");

    indicator_pos = Point(top_left.x() + ui_size.w() - indicator_width - 4, indicator_pos.y());
    m_minute_parity_indicator = new Ui::Indicator(indicator_pos.x(),
                                                  indicator_pos.y(),
                                                  indicator_width,
                                                  indicator_height,
                                                  Ui::Indicator::red_on,
                                                  Ui::Indicator::red_off,
                                                  "\nMinute\nparity");

    indicator_pos.translate(-indicator_width - indicator_spacing, 0);
    m_hour_parity_indicator = new Ui::Indicator(indicator_pos.x(),
                                                indicator_pos.y(),
                                                indicator_width,
                                                indicator_height,
                                                Ui::Indicator::red_on,
                                                Ui::Indicator::red_off,
                                                "\nHour\nparity");

    indicator_pos.translate(-indicator_width - indicator_spacing, 0);
    m_date_parity_indicator = new Ui::Indicator(indicator_pos.x(),
                                                indicator_pos.y(),
                                                indicator_width,
                                                indicator_height,
                                                Ui::Indicator::red_on,
                                                Ui::Indicator::red_off,
                                                "\nDate\nparity");

    indicator_pos.translate(-indicator_width - indicator_spacing, 0);
    m_call_indicator = new Ui::Indicator(indicator_pos.x(),
                                         indicator_pos.y(),
                                         indicator_width,
                                         indicator_height,
                                         Ui::Indicator::red_on,
                                         Ui::Indicator::red_off,
                                         "\nAbnormal\nTX operation");

    auto* spring = new Fl_Box(m_call_indicator->x() - 5, m_call_indicator->y(), 1, 1);
    indicators->resizable(spring);
    indicators->end();
    root->resizable(indicators);
    root->end();
    return root;
}

void Dcf77::on_marker_move(Hertz center_frequency) {
    m_mixer->set_frequency(center_frequency);
}

Pipe::Line<float, bool> Dcf77::build_pipeline() {
    IQMixer mixer(0);
    m_mixer = mixer.make_ref();

    return Pipe::line(std::move(mixer),
                      MovingAverage<Cmplx>(samples_per_bit),
                      Mapper<Cmplx, float>([](Cmplx cmplx) { return cmplx.magnitude(); }),
                      Normalizer(static_cast<WindowSize>(sample_rate * 2.2f), Normalizer::Lookahead::No, Normalizer::OffsetMode::Average),
                      Mapper<float, bool>([](float in) { return in > -.5f; }));
}

void Dcf77::process_pipeline_result(bool sample) {
    bool new_second = false;

    if (sample != m_last_level) {
        u32 bit_count = std::roundf(m_level_count / static_cast<float>(samples_per_bit));

        if (bit_count > 0) {
            m_level_count = bit_count;
            const auto state_info = info_for_state(m_state);
            set_status(state_info.description);
            if (m_state == State::WaitForMinuteMarker) {
                if (m_last_level && m_level_count > baud_rate * 1.5 && m_level_count <= 2 * baud_rate) {
                    m_state = State::ReadStartOfMinute;
                    m_bits_received = 0;
                    m_bits = 0;
                    m_seconds = -1;
                    m_time = m_receiving;
                    m_receiving = {};
                    update_minute();
                    m_tick = true;
                    logger().info() << "Detected minute marker";
                }
            } else {
                if (!m_last_level) {
                    m_bits <<= 1;

                    if (m_level_count == 2) {
                        m_bits |= 1;
                        m_parity = !m_parity;
                    } else if (m_level_count != 1) {
                        logger().info() << "Invalid bit received!";
                        m_state = State::WaitForMinuteMarker;
                        m_receiving = {};
                        if (Drtd::using_ui())
                            m_rx_fail_indicator->set_state(true);
                    }

                    if (state_info.point_of_read == m_bits_received) {
                        logger().info() << "Received in state " << static_cast<int>(m_state) << ": 0x" << std::hex << m_bits;
                        if ((m_state == State::ReadStartOfMinute && m_bits == 1) || (m_state == State::ReadStartOfTime && m_bits == 0)) {
                            logger().info() << "Did not receive expected marker bit!";
                            m_state = State::WaitForMinuteMarker;
                            m_receiving = {};
                            if (Drtd::using_ui())
                                m_rx_fail_indicator->set_state(true);
                        } else {
                            switch (m_state) {
                            case State::ReadStatus:
                                m_receiving.call = (m_bits & status_mask_call) != 0;
                                m_receiving.cet = (m_bits & status_mask_cet) != 0;
                                m_receiving.cest = (m_bits & status_mask_cest) != 0;

                                if ((m_bits & status_mask_leap_second) != 0)
                                    on_setup(); //When receiving a leap second announcement, just reset
                                break;
                            case State::ReadMinutes:
                                m_receiving.minute_parity_error = m_parity;
                                m_receiving.minutes = decode_bcd(m_bits >> 1, 7);
                                break;
                            case State::ReadHours:
                                m_receiving.hour_parity_error = m_parity;
                                m_receiving.hours = decode_bcd(m_bits >> 1, 6);
                                break;
                            case State::ReadDayOfMonth:
                                m_receiving.day_of_month = decode_bcd(m_bits, 6);
                                break;
                            case State::ReadDayOfWeek:
                                m_receiving.day_of_week = decode_bcd(m_bits, 3);
                                break;
                            case State::ReadMonthNumber:
                                m_receiving.month = decode_bcd(m_bits, 5);
                                break;
                            case State::ReadYear:
                                m_receiving.year = decode_bcd(m_bits, 8);
                                break;
                            case State::ReadDateParity:
                                m_receiving.date_parity_error = m_parity;
                                break;
                            default:
                                break;
                            }

                            m_state = next_state(m_state);
                            set_status(info_for_state(m_state).description);
                        }

                        m_bits = 0;
                        if (!state_info.date_state)
                            m_parity = false;
                    }

                    ++m_bits_received;
                    if (m_bits_received == 59) {
                        logger().info() << "Successfully received!";
                        if (Drtd::using_ui())
                            m_rx_fail_indicator->set_state(false);

                        m_state = State::WaitForMinuteMarker;
                    }

                    if (Drtd::using_ui())
                        m_rx_indicator->set_state(m_state != State::WaitForMinuteMarker);
                }
            }

            if (!m_last_level) {
                /* Sanity check */
                const auto current = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(current - m_last_update).count() > 500) {
                    m_last_update = current;
                    advance_time();
                    m_tick = true;
                    new_second = true;
                }
            }

            m_level_count = 0;
        }

        m_last_level = sample;
    }

    ++m_level_count;
    if ((tick_time() || new_second) && !Drtd::using_ui()) {
        const bool time_valid = (m_time.cest && !m_time.cet) || (!m_time.cest && m_time.cet);
        std::ostringstream builder;

        builder << create_date_string() << " - " << create_time_string() << " ; ";
        if (!time_valid || m_time.date_parity_error || m_time.hour_parity_error || m_time.minute_parity_error)
            builder << 'E';

        if (m_time.cet)
            builder << " CET";

        if (m_time.cest)
            builder << " CEST";

        puts(builder.str().c_str());
    }
}

bool Dcf77::tick_time() {
    if (!m_tick)
        return false;

    ++m_ticks;
    if (m_ticks >= sample_rate * 1.1f) {
        advance_time();
        return true;
    }

    return false;
}

void Dcf77::advance_time() {
    if (!m_tick)
        return;

    m_ticks = 0;
    m_tick = false;
    ++m_seconds;
    m_seconds %= 60;

    if (Drtd::using_ui())
        m_time_label->copy_label(create_time_string().c_str());
}

Dcf77::State Dcf77::next_state(Dcf77::State state) {
    return static_cast<State>((static_cast<u8>(state) + 1) % static_cast<u8>(State::__Count));
}

void Dcf77::update_minute() {
    if (!Drtd::using_ui())
        return;

    m_date_label->copy_label(create_date_string().c_str());
    m_minute_parity_indicator->set_state(m_time.minute_parity_error);
    m_hour_parity_indicator->set_state(m_time.hour_parity_error);
    m_date_parity_indicator->set_state(m_time.date_parity_error);
    m_cest_indicator->set_state(m_time.cest);
    m_cet_indicator->set_state(m_time.cet);
    m_call_indicator->set_state(m_time.call);
}

std::string Dcf77::create_time_string() const {
    std::ostringstream builder;
    builder.fill('0');
    builder << std::setw(2)
            << static_cast<int>(m_time.hours)
            << ":" << std::setw(2)
            << static_cast<int>(m_time.minutes)
            << ":" << std::setw(2)
            << static_cast<int>(m_seconds);
    return builder.str();
}

std::string Dcf77::create_date_string() const {
    std::ostringstream builder;
    builder.fill('0');
    builder << dow_lookup[m_time.day_of_week] << ", ";

    if (m_time.day_of_month)
        builder << std::setw(2) << static_cast<int>(m_time.day_of_month);
    else
        builder << "--";

    builder << '.';
    if (m_time.month)
        builder << std::setw(2) << static_cast<int>(m_time.month);
    else
        builder << "--";

    builder << '.';
    if (m_time.year)
        builder << "20" << std::setw(2) << static_cast<int>(m_time.year);
    else
        builder << "----";

    return builder.str();
}

Dcf77::StateInfo Dcf77::info_for_state(Dcf77::State state) {

    switch (state) {
    case State::WaitForMinuteMarker:
        return { .point_of_read = 0, .description = "Waiting for begin of minute...", .date_state = false };
    case State::ReadStartOfMinute:
        return { .point_of_read = 0, .description = "Reading start of minute...", .date_state = false };
    case State::ReadCivilWarning:
        return { .point_of_read = 14, .description = "Reading civil warning bits...", .date_state = false };
    case State::ReadStatus:
        return { .point_of_read = 19, .description = "Reading status bits...", .date_state = false };
    case State::ReadStartOfTime:
        return { .point_of_read = 20, .description = "Reading start of time bit...", .date_state = false };
    case State::ReadMinutes:
        return { .point_of_read = 28, .description = "Reading minute...", .date_state = false };
    case State::ReadHours:
        return { .point_of_read = 35, .description = "Reading hour...", .date_state = false };
    case State::ReadDayOfMonth:
        return { .point_of_read = 41, .description = "Reading day of month...", .date_state = true };
    case State::ReadDayOfWeek:
        return { .point_of_read = 44, .description = "Reading day of week...", .date_state = true };
    case State::ReadMonthNumber:
        return { .point_of_read = 49, .description = "Reading month...", .date_state = true };
    case State::ReadYear:
        return { .point_of_read = 57, .description = "Reading year...", .date_state = true };
    case State::ReadDateParity:
        return { .point_of_read = 58, .description = "Reading date parity...", .date_state = true };
    default:
        assert(false);
        Util::should_not_be_reached();
    }
}

Util::Buffer<std::string> Dcf77::changeable_parameters() const {
    return { "Center frequency (Int)" };
}

bool Dcf77::setup_parameters(const Util::Buffer<std::string>& parameters) {
    int center_frequency = Util::parse_int(parameters[0]).value_or(-1);
    if (center_frequency < 0) {
        puts("Invalid center frequency");
        return false;
    }

    set_center_frequency(center_frequency);
    return true;
}
