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
#include "Indicator.hpp"
#include <Fl/fl_draw.H>
#include <util/Size.hpp>
#include <util/Util.hpp>

using namespace Ui;

static constexpr Size indicator_size(25, 15);

Indicator::Indicator(int x, int y, int w, int h, Fl_Color on_color, Fl_Color off_color, const char* label)
    : Fl_Group(x, y, w, h)
    , m_indicator(x + Util::center(w, indicator_size.w()), y + 4, indicator_size.w(), indicator_size.h())
    , m_label(x, m_indicator.y() + m_indicator.h() + 2, w, 13, label)
    , m_on_color(on_color)
    , m_off_color(off_color) {
    m_indicator.box(FL_DOWN_BOX);
    m_label.labelsize(13);
    m_label.labelfont(FL_BOLD);
    resizable(nullptr);
    m_indicator.color(m_off_color);
    end();
}

void Indicator::set_state(bool state) {
    if (state == m_state)
        return;

    m_state = state;
    m_indicator.color(m_state ? m_on_color : m_off_color);
    m_indicator.redraw();
    m_indicator.damage(FL_DAMAGE_ALL);
}
