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
#include "Layer.hpp"
#include <algorithm>
#include <cassert>

using namespace Ui;

LayerDraw::LayerDraw(std::shared_ptr<Layer> layer)
    : m_layer(layer) {
    m_layer->begin_draw();
}

LayerDraw::~LayerDraw() {
    if (m_layer)
        m_layer->end_draw();
}

LayerDraw::LayerDraw(LayerDraw&& other) {
    std::swap(m_layer, other.m_layer);
}

void Layer::begin_draw() {
    if (!m_buffer_valid || m_in_draw)
        return;

    m_draw_lock.lock();
    m_old_window = fl_window;
    fl_window = m_offscreen_buffer;
    m_surface = Fl_Surface_Device::surface();
    Fl_Display_Device::display_device()->set_current();
    fl_push_no_clip();
    m_in_draw = true;
}

void Layer::end_draw() {
    if (!m_buffer_valid || !m_in_draw)
        return;

    fl_pop_clip();
    fl_window = m_old_window;
    m_surface->set_current();
    m_in_draw = false;
    m_draw_lock.unlock();
}

void Layer::draw(u32 real_x, u32 real_y) {
    if (!m_buffer_valid)
        return;

    m_draw_lock.lock();
    fl_copy_offscreen(real_x + m_x, real_y + m_y, m_buffer_width, m_buffer_height, m_offscreen_buffer, 0, 0);
    m_draw_lock.unlock();
}

void Layer::resize_if_needed(u32 parent_width, u32 parent_height) {
    int real_width = (m_width == parent_size) ? parent_width - m_x : m_width;
    int real_height = (m_height == parent_size) ? parent_height - m_y : m_height;

    if (real_width <= 0 || real_height <= 0)
        return;

    u32 real_width_u = static_cast<u32>(real_width);
    u32 real_height_u = static_cast<u32>(real_height);
    bool has_to_resize = m_buffer_width != real_width_u || m_buffer_height != real_height_u;
    m_buffer_width = real_width;
    m_buffer_height = real_height;

    if (m_buffer_valid && has_to_resize) {
        Fl_Offscreen old = m_offscreen_buffer;
        m_offscreen_buffer = fl_create_offscreen(real_width, real_height);
        begin_draw();
        fl_rectf(0, 0, real_width, real_height, m_clear_color);
        fl_copy_offscreen(0, 0, real_width, real_height, old, 0, 0);
        end_draw();
        fl_delete_offscreen(old);
    } else if (!m_buffer_valid) {
        m_offscreen_buffer = fl_create_offscreen(real_width, real_height);
        m_buffer_valid = true;
        begin_draw();
        fl_rectf(0, 0, real_width, real_height, m_clear_color);
        end_draw();
    }
}
