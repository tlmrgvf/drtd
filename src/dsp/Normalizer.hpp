#pragma once

#include <Fl/fl_draw.H>
#include <cmath>
#include <limits>
#include <pipe/Component.hpp>
#include <util/RingBuffer.hpp>
#include <util/Types.hpp>

namespace Dsp {

class Normalizer final : public RefableComponent<float, float, Normalizer> {
public:
    enum class Lookahead : bool {
        Yes,
        No
    };

    enum class OffsetMode : bool {
        Minimum,
        Average
    };

    Normalizer(WindowSize, Lookahead, OffsetMode);

    virtual Size calculate_size() override;

    void set_window_size(WindowSize);
    WindowSize window_size() const { return m_window_size; }

protected:
    virtual Normalizer& ref() override { return *this; }
    virtual void draw_at(Point) override;
    virtual float process(float) override;

private:
    WindowSize m_window_size;
    Lookahead m_lookahead;
    OffsetMode m_offset_mode;
    RingBuffer<float> m_delay_buffer;
    float m_factor { 1 };
    float m_average { 0 };
    float m_offset { 0 };
    float m_count { 0 };
    float m_max { std::numeric_limits<float>::min() };
    float m_min { std::numeric_limits<float>::max() };
};

}
