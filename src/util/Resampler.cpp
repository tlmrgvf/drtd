#include "Resampler.hpp"

#include <cassert>

using namespace Util;

Resampler::Resampler(float source_rate, float target_rate)
    : m_target_ratio(source_rate / target_rate)
    , m_down_sample(source_rate > target_rate) {
    assert(source_rate > 0 && target_rate > 0);
    if (m_down_sample)
        m_lowpass_filter = std::make_unique<Dsp::Biquad::Filter<float>>(Dsp::Biquad::Type::Lowpass, source_rate, target_rate / 2, Dsp::Biquad::invsqrt2);
}

void Resampler::process_input_sample(float sample) {
    if (m_down_sample) {
        sample = m_lowpass_filter->filter_sample(sample);
        m_average += sample;
        ++m_averaged_values;
    }

    m_current_sample = sample;
    ++m_current_ratio;
}

bool Resampler::read_output_sample(float& result) {
    if (m_current_ratio >= 0) {
        m_current_ratio -= m_target_ratio;

        if (m_down_sample) {
            if (m_averaged_values == 0) {
                result = 0;
                return true;
            }

            result = m_average / static_cast<float>(m_averaged_values);
            m_averaged_values = 0;
            m_average = 0;
            return true;
        } else {
            result = m_current_sample;
            return true;
        }
    }

    return false;
}
