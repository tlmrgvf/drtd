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

#include <FL/Fl_Spinner.H>
#include <FL/Fl_Window.H>
#include <pipe/Component.hpp>
#include <ui/component/FrequencyPlot.hpp>
#include <util/Singleton.hpp>

namespace Dsp {
class MovingAverageBase;
}

namespace Ui {

class MovingAverageDialog final : Fl_Window {
public:
    static void show_dialog(ConfigRef<Dsp::MovingAverageBase> filter);
    static void close_dialog();
    static void update_dialog();

private:
    static MovingAverageDialog* new_dialog() { return new MovingAverageDialog(); }
    static inline Singleton<MovingAverageDialog, MovingAverageDialog::new_dialog> s_moving_average_dialog;

    MovingAverageDialog();

    static void update_filter(Fl_Widget*);

    Fl_Group* m_plot_container { nullptr };
    FrequencyPlot* m_plot { nullptr };
    Fl_Group* m_settings_container { nullptr };
    Fl_Spinner* m_taps { nullptr };
};

}
