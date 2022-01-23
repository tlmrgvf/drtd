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

#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>

namespace Ui {

class Indicator : public Fl_Group {
public:
    static constexpr Fl_Color yellow_off { 0x50160000 };
    static constexpr Fl_Color yellow_on { 0xDDCC0000 };
    static constexpr Fl_Color green_off { 0x00300000 };
    static constexpr Fl_Color green_on { 0x00CC0000 };
    static constexpr Fl_Color red_off { 0x4C000000 };
    static constexpr Fl_Color red_on { 0xC7000000 };

    Indicator(int x, int y, int w, int h, Fl_Color on_color, Fl_Color off_color, const char* label);

    void set_state(bool state);

private:
    Fl_Box m_indicator;
    Fl_Box m_label;
    bool m_state { false };
    Fl_Color m_on_color;
    Fl_Color m_off_color;
};

}
