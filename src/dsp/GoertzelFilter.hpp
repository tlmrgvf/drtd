#pragma once
#include <pipe/Component.hpp>
#include <util/RingBuffer.hpp>

namespace Dsp {
class GoertzelFilter final : public ComponentBase<float, float> {
public:
    GoertzelFilter(Taps taps, float frequency);

    virtual Size calculate_size() override;
    virtual void draw_at(Point) override;

protected:
    virtual SampleRate on_init(SampleRate, int&) override;
    virtual float process(float) override;

private:
    Taps m_taps;
    float m_frequency;
    RingBuffer<float> m_buffer;
    float m_coefficient { 0 };
};

}
