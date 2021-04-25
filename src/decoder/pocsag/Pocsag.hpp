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

#include "PocsagData.hpp"
#include "PocsagMessage.hpp"
#include <Fl/Fl_Choice.H>
#include <decoder/Decoder.hpp>
#include <dsp/BitConverter.hpp>
#include <dsp/FirFilter.hpp>
#include <dsp/MovingAverage.hpp>
#include <ui/component/Indicator.hpp>
#include <ui/component/TextDisplay.hpp>
#include <util/BitBuffer.hpp>
#include <util/CallbackManager.hpp>
#include <util/bch/BchCode.hpp>

namespace Dsp {

class Pocsag final : public Decoder<bool> {
public:
    Pocsag();
    virtual bool setup_parameters(const Util::Buffer<std::string>&) override;
    virtual Util::Buffer<std::string> changeable_parameters() const override;

protected:
    virtual void on_setup() override;
    virtual void on_tear_down() override;
    virtual Pipe::Line<float, bool> build_pipeline() override;
    virtual Fl_Widget* build_ui(Util::Point top_left, Util::Size ui_size) override;
    virtual void process_pipeline_result(bool) override;

private:
    enum class State : u8 {
        FirstBitSinceSync,
        WaitForInitialSyncWord,
        WaitForImmediateSyncWord,
        ReadPocsagBatch
    };

    static std::string state_string(State);

    void reset(bool);
    void update_state(State);
    void message_done();

    ConfigRef<MovingAverageBase> m_matched_filter;
    ConfigRef<BitConverter> m_converter;

    BitBuffer<Util::PushSequence::MsbPushedFirst, PocsagProtocol::Data::codeword_bit_count> m_incoming_buffer;
    u32 m_preamble_count { 0 };
    u32 m_codeword_count { 0 };
    bool m_received_parity { false };
    bool m_inverted { false };
    bool m_last_bit { false };
    State m_state { State::FirstBitSinceSync };
    Bch::Code<Bch::EncodingType::Prefix, 31, 21, 2> m_bch_code { Bch::Z2Polynomial(0b11101101001) };
    PocsagProtocol::MessageBuilder m_message_builder;
    PocsagProtocol::Message::ContentType m_content_type { PocsagProtocol::Message::ContentType::AlphaNumeric };
    CallbackManager m_callback_manager;
    Ui::Indicator* m_sync_512_indicator { nullptr };
    Ui::Indicator* m_sync_1200_indicator { nullptr };
    Ui::Indicator* m_sync_2400_indicator { nullptr };
    Ui::Indicator* m_data_indicator { nullptr };
    Ui::TextDisplay* m_text_box { nullptr };
    Fl_Choice* m_content_selector { nullptr };
};

}
