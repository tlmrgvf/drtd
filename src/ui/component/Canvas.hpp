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

#include "Layer.hpp"
#include <FL/Fl_Box.H>
#include <functional>
#include <memory>
#include <util/Types.hpp>
#include <vector>

namespace Ui {

class Canvas : public Fl_Box {
public:
    Canvas(u32 x, u32 y, u32 w, u32 h, u16 padding)
        : Fl_Box(x, y, w, h)
        , m_padding(padding) {
    }

    std::shared_ptr<Layer> make_layer(u16 x, u16 y, u16 width, u16 height, Fl_Color clear_color = fl_rgb_color(0, 0, 0), bool initialize = false);
    void resize_offscreen_buffers();
    u16 padding() const { return m_padding; }

private:
    u32 content_width() const { return std::max(w() - 2 * m_padding, 0); }
    u32 content_height() const { return std::max(h() - 2 * m_padding, 0); }

    virtual void draw() override;

    u16 m_padding;
    std::vector<std::shared_ptr<Layer>> m_layers;
};

}
