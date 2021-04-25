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

#include <Fl/Fl_RGB_Image.H>
#include <functional>
#include <memory>
#include <string>
#include <util/Types.hpp>

namespace Ui {
class MainGui;
}

namespace Util {
template<typename T>
class Buffer;
}

namespace Dsp {
class DecoderBase;
}

namespace Drtd {

struct AudioLine {
    u8 index { 0 };
    std::string name { "" };
    std::string description { "" };
};

Fl_RGB_Image* drtd_icon();
u8 active_decoder_index();
bool using_ui();
Ui::MainGui& main_gui();
bool use_decoder(u8 index);
bool start_processing(u8 index);
void stop_processing();
const Drtd::AudioLine& default_audio_line();
bool switch_audio_line(u8 audio_line_index);
u8 current_audio_line();
const Util::Buffer<Drtd::AudioLine>& audio_lines();
void for_each_decoder(std::function<void(Dsp::DecoderBase&)> callback);
std::shared_ptr<Dsp::DecoderBase> active_decoder();
void monitor_sample(float sample);

}
