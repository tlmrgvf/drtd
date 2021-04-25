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
#include <Fl/Fl_Light_Button.H>
#include <Fl/Fl_Slider.H>
#include <Fl/Fl_Window.H>
#include <stdio.h>
#include <util/Singleton.hpp>

namespace Ui {

class ScopeDialog final : Fl_Window {
public:
    static void show_dialog();
    static void load_from_scope();
    static void save_to_scope(Fl_Widget*, void*);

private:
    ScopeDialog();

    static ScopeDialog* new_dialog() { return new ScopeDialog(); };

    static inline Singleton<ScopeDialog, ScopeDialog::new_dialog> s_scope_dialog;

    Fl_Slider m_zoom;
    Fl_Button m_reset_zoom;
    Fl_Check_Button m_remove_bias;
    Fl_Check_Button m_normalized;
    Fl_Light_Button m_pause;
    Fl_Button m_capture;
};

}
