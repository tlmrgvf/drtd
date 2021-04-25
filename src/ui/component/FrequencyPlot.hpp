#pragma once

#include "Canvas.hpp"
#include <util/Buffer.hpp>
#include <util/Size.hpp>

namespace Ui {

class FrequencyPlot final : public Canvas {
public:
    FrequencyPlot(u32 x, u32 y, u32 w, u32 h);

    void plot(Buffer<float> values, float max_frequency, float min_value, float max_value, float reference = 1);
    void plot();
    virtual int handle(int) override;

private:
    static constexpr unsigned db_scale_max = 200;
    static constexpr unsigned db_scale_min = 5;
    static constexpr unsigned db_scale_step = 5;
    static constexpr u8 frequency_divisions = 20;
    static constexpr u8 amplitude_divisions = 10;

    float m_max_frequency { 1 };
    Buffer<float> m_values;
    float m_min_value { 1 };
    float m_max_value { 1 };
    float m_reference { 1 };
    u32 m_vertical_scale { db_scale_max };
    std::shared_ptr<Layer> m_plot_layer;
    unsigned m_marker_x { 0 };
    bool m_plot_linear { false };
    bool m_show_marker { false };
    Size m_old_size;
};

}
