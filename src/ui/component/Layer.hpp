#pragma once

#include <Fl/Fl_Device.H>
#include <Fl/fl_draw.H>
#include <Fl/x.H>
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
