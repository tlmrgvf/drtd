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

#include <Fl/Fl_Check_Button.H>
#include <Fl/Fl_Choice.H>
#include <Fl/Fl_Slider.H>
#include <Fl/Fl_Spinner.H>
#include <Fl/Fl_Window.H>
#include <ui/component/Waterfall.hpp>
#include <util/Singleton.hpp>

namespace Ui {

class WaterfallDialog final : Fl_Window {
public:
    static constexpr float s_min_zoom { -10 };
    static constexpr float s_max_zoom { 10 };
    static void show_dialog();
    static void load_from_waterfall(const Waterfall::Settings& settings);

private:
    WaterfallDialog();

    static WaterfallDialog* new_dialog() { return new WaterfallDialog(); };
    static void save_to_waterfall(Fl_Widget*, void*);

    void update_offset_spinner_limits(const Waterfall::Settings& settings);

    static inline Singleton<WaterfallDialog, WaterfallDialog::new_dialog> s_waterfall_dialog;

    Fl_Choice m_window;
    Fl_Slider m_zoom;
    Fl_Button m_reset_zoom;
    Fl_Spinner m_bin_offset;
    Fl_Slider m_speed_multiplier;
    Fl_Choice m_bins;
    Fl_Choice m_palette;
    Fl_Check_Button m_power_spectrum;
};

}
