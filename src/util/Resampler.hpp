#pragma once

#include <dsp/Biquad.hpp>
#include <memory>
#include <optional>

namespace Util {

class Resampler final {
public:
    Resampler(float source_rate, float target_rate);

    void process_input_sample(float sample);
    bool read_output_sample(float& result);

private:
    const float m_target_ratio;
    const bool m_down_sample;
    std::unique_ptr<Dsp::Biquad::Filter<float>> m_lowpass_filter { nullptr };
    float m_current_ratio { 0 };
    float m_average { 0 };
    float m_current_sample { 0 };
    u32 m_averaged_values { 0 };
};

}
