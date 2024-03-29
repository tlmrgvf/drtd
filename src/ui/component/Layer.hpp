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

#include <FL/Fl_Device.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <util/Types.hpp>

namespace Ui {

class Layer;

class LayerDraw final {
public:
    LayerDraw(std::shared_ptr<Layer> layer);
    ~LayerDraw();
    LayerDraw(LayerDraw&) = delete;
    LayerDraw(LayerDraw&&);

private:
    std::shared_ptr<Layer> m_layer;
};

class Layer final {
public:
    friend class Canvas;
    friend class LayerDraw;
    static constexpr u16 parent_size = 0xFFFF;

    u32 x() const { return m_x; };
    u32 y() const { return m_y; };

    void set_x(u32 x) { m_x = x; };
    void set_y(u32 y) { m_y = y; };

    void set_width(u32 width) { m_width = width; };
    void set_height(u32 height) { m_height = height; };

    Layer(u32 x, u32 y, u32 width, u32 height, Fl_Color clear_color)
        : m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height)
        , m_clear_color(clear_color) {
    }

    ~Layer() {
        if (m_buffer_valid)
            fl_delete_offscreen(m_offscreen_buffer);
    }

    u32 current_width() const { return m_buffer_width; }
    u32 current_height() const { return m_buffer_height; }
    Fl_Color clear_color() const { return m_clear_color; }
    Fl_Offscreen offscreen_buffer() const { return m_offscreen_buffer; }

private:
    Layer(Layer&) = delete;
    Layer(Layer&&) = delete;

    void begin_draw();
    void end_draw();

    void resize_if_needed(u32 parent_widh, u32 parent_height);
    void draw(u32 real_x, u32 real_y);

    constexpr u32 real_width(u32 parent_width) const { return (m_width == parent_size) ? parent_width - m_x : m_width; }
    constexpr u32 real_height(u32 parent_height) const { return (m_height == parent_size) ? parent_height - m_y : m_height; }

    u32 m_x;
    u32 m_y;
    u32 m_width;
    u32 m_height;
    u32 m_buffer_width { 0 };
    u32 m_buffer_height { 0 };
    Fl_Offscreen m_offscreen_buffer { 0 };
    bool m_buffer_valid { false };
    bool m_in_draw { false };
    Window m_old_window;
    Fl_Surface_Device* m_surface;
    Fl_Color m_clear_color;
    std::mutex m_draw_lock;
};
}
