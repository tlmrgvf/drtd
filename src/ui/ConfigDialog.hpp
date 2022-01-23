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

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Window.H>
#include <functional>
#include <sstream>
#include <ui/component/Canvas.hpp>
#include <util/Point.hpp>

namespace Ui {

class ConfigDialog final : Fl_Window {
public:
    static void show_dialog();
    static void refresh();

    ConfigDialog();

private:
    class PipelineCanvas final : public Canvas {
    public:
        PipelineCanvas(u32 x, u32 y, u32 w, u32 h, u8 padding)
            : Canvas(x, y, w, h, padding) {
        }

        virtual int handle(int event) override {
            if (event == FL_PUSH && m_on_click)
                m_on_click({ Fl::event_x() - x() - padding(), Fl::event_y() - y() - padding() }, Fl::event_button(), Fl::event_ctrl());
            return event;
        }

        std::function<void(Util::Point, int, bool)> m_on_click;
    };

    void update(bool reposition);
    void pipeline_click_event(Util::Point, int, bool);

    Fl_Choice m_audio_line;
    Fl_Choice m_monitored_value;
    PipelineCanvas m_pipeline_canvas;
    std::shared_ptr<Layer> m_pipeline_layer;
    Fl_Button m_reset_button;
};

}
