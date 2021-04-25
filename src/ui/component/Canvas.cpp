#include "Canvas.hpp"

using namespace Ui;

void Canvas::draw() {
    Fl_Box::draw();

    if (content_width() < 2 * m_padding || content_height() < 2 * m_padding)
        return;

    u32 real_x = x() + m_padding;
    u32 real_y = y() + m_padding;

    fl_push_clip(real_x, real_y, content_width(), content_height());
    for (const auto& layer : m_layers) {
        layer->resize_if_needed(content_width(), content_height());
        layer->draw(real_x, real_y);
    }
    fl_pop_clip();
}

void Canvas::resize_offscreen_buffers() {
    for (const auto& layer : m_layers)
        layer->resize_if_needed(content_width(), content_height());
}

std::shared_ptr<Layer> Canvas::make_layer(u16 x, u16 y, u16 width, u16 height, Fl_Color clear_color, bool initialize) {
    auto layer = std::make_shared<Layer>(x, y, width, height, clear_color);
    m_layers.push_back(layer);
    if (initialize)
        layer->resize_if_needed(content_width(), content_height());
    return layer;
}
