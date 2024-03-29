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
#pragma once

#include <chrono>
#include <decoder/Decoder.hpp>
#include <dsp/IQMixer.hpp>
#include <ui/component/Indicator.hpp>
#include <util/SNRCalculator.hpp>

namespace Dsp {

class Dcf77 final : public Decoder<bool> {
public:
    Dcf77();

protected:
    virtual Pipe::Line<float, bool> build_pipeline() override;
    virtual Fl_Widget* build_ui(Point, Size) override;
    virtual void process_pipeline_result(bool) override;
    virtual void on_marker_move(Hertz) override;
    virtual void on_setup() override;
    virtual Util::Buffer<std::string> changeable_parameters() const override;
    virtual bool setup_parameters(const Util::Buffer<std::string>&) override;

private:
    enum class State : u8 {
        WaitForMinuteMarker,
        ReadStartOfMinute,
        ReadCivilWarning,
        ReadStatus,
        ReadStartOfTime,
        ReadMinutes,
        ReadHours,
        ReadDayOfMonth,
        ReadDayOfWeek,
        ReadMonthNumber,
        ReadYear,
        ReadDateParity,
        __Count
    };

    struct TimeInfo {
        TimeInfo()
            : call(false)
            , cet(false)
            , cest(false)
            , hour_parity_error(false)
            , minute_parity_error(false)
            , date_parity_error(false) {
        }

        u8 minutes { 0 };
        u8 hours { 0 };
        u8 day_of_week { 0 };
        u8 day_of_month { 0 };
        u8 year { 0 };
        u8 month { 0 };
        bool call : 1;
        bool cet : 1;
        bool cest : 1;
        bool hour_parity_error : 1;
        bool minute_parity_error : 1;
        bool date_parity_error : 1;
    };

    struct StateInfo {
        u8 point_of_read;
        std::string description;
        bool date_state;
    };

    static StateInfo info_for_state(State);
    static State next_state(State);
    bool tick_time();
    void update_minute();
    void advance_time();
    std::string create_date_string() const;
    std::string create_time_string() const;

    ConfigRef<IQMixer> m_mixer;
    Util::SNRCalculator m_snr_calculator;
    State m_state { State::WaitForMinuteMarker };
    TimeInfo m_time;
    TimeInfo m_receiving;
    bool m_last_level { false };
    bool m_parity;
    bool m_tick { false };
    u32 m_level_count { 0 };
    std::chrono::steady_clock::time_point m_last_update;
    u8 m_bits_received { 0 };
    u32 m_bits { 0 };
    u32 m_ticks { 0 };
    i8 m_seconds { 0 };
    Fl_Box* m_time_label { nullptr };
    Fl_Box* m_date_label { nullptr };
    Ui::Indicator* m_minute_parity_indicator { nullptr };
    Ui::Indicator* m_hour_parity_indicator { nullptr };
    Ui::Indicator* m_date_parity_indicator { nullptr };
    Ui::Indicator* m_cest_indicator { nullptr };
    Ui::Indicator* m_cet_indicator { nullptr };
    Ui::Indicator* m_call_indicator { nullptr };
    Ui::Indicator* m_rx_indicator { nullptr };
    Ui::Indicator* m_rx_fail_indicator { nullptr };
};

}
