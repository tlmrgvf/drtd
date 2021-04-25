/*
BSD 2-Clause License

Copyright (c) 2020, Till Mayer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "Resampler.hpp"

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
