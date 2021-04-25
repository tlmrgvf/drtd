#pragma once

#include "Layer.hpp"
#include <Fl/Fl_Box.H>
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
