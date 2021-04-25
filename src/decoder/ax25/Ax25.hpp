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

#include "Packet.hpp"
#include <Fl/Fl_Text_Buffer.H>
#include <decoder/Decoder.hpp>
#include <list>
#include <sstream>
#include <ui/component/Indicator.hpp>
#include <ui/component/TextDisplay.hpp>
#include <util/BitBuffer.hpp>
#include <util/CallbackManager.hpp>

namespace Dsp {

class Ax25 final : public Decoder<bool> {
public:
    Ax25();

protected:
    virtual Fl_Widget* build_ui(Point top_left, Size ui_size) override;
    virtual Pipe::Line<float, bool> build_pipeline() override;
    virtual void process_pipeline_result(bool) override;
    virtual void on_setup() override;

private:
    static constexpr SampleRate sample_rate = 22050;
    static constexpr BaudRate baud_rate = 1200;
    static constexpr u8 headers_needed = 5;

    enum class State : u8 {
        WaitFlag,
        CountFlag,
        WaitData,
        WaitEnd
    };

    struct StateInfo {
        bool ignore_stuffed_bits { false };
        bool update_indicator { false };
        const char* label { "" };
    };

    static StateInfo info_for_state(State);
    void change_state_to(State);
    void packet_done();

    BitBuffer<Util::PushSequence::LsbPushedFirst, 8> m_in_buffer;
    BitBuffer<Util::PushSequence::LsbPushedFirst, 8> m_delay_buffer;
    BitBuffer<Util::PushSequence::LsbPushedFirst, 8> m_processed_buffer;
    unsigned m_one_count { 0 };
    unsigned m_header_count { 0 };
    State m_state { State::WaitFlag };
    std::vector<uint8_t> m_packet_buffer;
    Ui::TextDisplay* m_text_box { nullptr };
    Ui::Indicator* m_data_indicator { nullptr };
    Ui::Indicator* m_sync_indicator { nullptr };
    StateInfo m_current_state_info;
    CallbackManager m_callback_manager;
};

}
