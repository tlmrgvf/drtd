#pragma once

#include "Canvas.hpp"
#include <util/Buffer.hpp>
#include <util/Cmplx.hpp>

namespace Ui {

class XYScope final : public Canvas {
public:
    XYScope(int x, int y, int size, WindowSize window_size, float fade_factor, bool connect);
    void process(Cmplx);

private:
    std::shared_ptr<Ui::Layer> m_draw_layer;
    float m_fade_factor;
    Buffer<Cmplx> m_sample_buffer;
    bool m_connect;

    WindowSize m_sample_count { 0 };
    float m_max_magnitude { 0 };
};

}
