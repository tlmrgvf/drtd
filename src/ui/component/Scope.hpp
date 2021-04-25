#pragma once

#include "Canvas.hpp"
#include <Fl/Fl.H>
#include <limits>
#include <util/Buffer.hpp>
#include <util/Limiter.hpp>

namespace Ui {

class Scope final : public Canvas {
public:
    struct Settings {
        i8 zoom { 0 };
        bool remove_dc_bias { true };
        bool normalized { false };
    };

    Scope(int x, int y, int w, int h)
        : Canvas(x, y, w, h, 2)
        , m_layer(make_layer(0, 0, Layer::parent_size, Layer::parent_size)) {
        box(FL_DOWN_BOX);
    }

    void process_sample(float sample);
    Settings& settings() { return m_settings; }
    void single_shot() { m_single_shot = true; }
    void set_paused(bool paused) { m_paused = paused; }

private:
    virtual int handle(int event) override;
    void recalculate_bias();
    float real_bias() { return m_settings.remove_dc_bias ? m_dc_bias : 0; }

    std::shared_ptr<Layer> m_layer;
    Settings m_settings;
    Util::Limiter m_limiter { 30 };
    Util::Buffer<float> m_samples;
    u32 m_width { 1 };
    u32 m_samples_captured { 0 };
    u32 m_samples_averaged { 0 };
    float m_sample_average { 0 };
    float m_sample_max { std::numeric_limits<float>::min() };
    float m_sample_min { std::numeric_limits<float>::max() };
    float m_dc_bias { 0 };
    float m_last_sample { 0 };
    bool m_single_shot { false };
    bool m_paused { false };
};
}
