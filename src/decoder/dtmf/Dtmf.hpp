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

#include <decoder/Decoder.hpp>
#include <ui/component/Indicator.hpp>
#include <ui/component/TextDisplay.hpp>
#include <util/CallbackManager.hpp>

namespace Dsp {

class Dtmf final : public Decoder<u8> {
public:
    Dtmf();

protected:
    virtual Pipe::Line<float, u8> build_pipeline() override;
    virtual Fl_Widget* build_ui(Point, Size) override;
    virtual void process_pipeline_result(u8) override;

private:
    char m_last_symbol { '-' };
    char m_last_valid { '-' };
    Samples m_sample_count { 0 };
    Samples m_last_interruption_length { 0 };
    Samples m_samples_since_last_valid_symbol { 0 };
    Ui::TextDisplay* m_text_box { nullptr };
    Ui::Indicator* m_detect_indicator { nullptr };
    Util::CallbackManager m_callback_manager;
};

}
