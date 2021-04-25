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
