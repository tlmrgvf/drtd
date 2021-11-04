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
#include "Biquad.hpp"
#include <cassert>
#include <util/Util.hpp>

namespace Dsp::Biquad {
/*
 * Formulas: https://www.w3.org/2011/audio/audio-eq-cookbook.html
 * Good explanation of biquad filters: https://arachnoid.com/BiQuadDesigner/index.html
 *                                                      and
 *                                     https://www.earlevel.com/main/2003/02/28/biquads/
 */

std::string Coefficients::name_for(Type type) {
    return names[static_cast<size_t>(type)];
}

std::string Coefficients::parameter_description_for(Type type) {
    return parameter_descriptions[static_cast<size_t>(type)];
}

Coefficients::Coefficients(Type type, SampleRate sample_rate, float center, float parameter)
    : m_sample_rate(sample_rate) {
    assert(sample_rate);

    float w0 = Util::two_pi_f * (center / sample_rate);
    float cos_w0 = std::cos(w0);
    const float alpha_q = std::sin(w0) / (2 * parameter);
    const float alpha_bandwidth = sinf(w0) * sinhf(logf(2) / 2 * parameter * (w0 / sinf(w0)));

    switch (type) {
    case Type::Lowpass:
        input_0 = (1 - cos_w0) / 2;
        input_1 = 1 - cos_w0;
        input_2 = input_0;

        feedback_0 = 1 + alpha_q;
        feedback_1 = -2 * cos_w0;
        feedback_2 = 1 - alpha_q;
        break;
    case Type::Highpass:
        input_0 = (1 + cos_w0) / 2;
        input_1 = -1 - cos_w0;
        input_2 = input_0;

        feedback_0 = 1 + alpha_q;
        feedback_1 = -2 * cos_w0;
        feedback_2 = 1 - alpha_q;
        break;
    case Type::BandpassPeak:
        input_0 = alpha_bandwidth;
        input_1 = 0;
        input_2 = -alpha_bandwidth;

        feedback_0 = 1 + alpha_bandwidth;
        feedback_1 = -2 * cos_w0;
        feedback_2 = 1 - alpha_bandwidth;
        break;
    case Type::BandpassSkirt:
        input_0 = std::sin(w0) / 2;
        input_1 = 0;
        input_2 = -input_0;

        feedback_0 = 1 + alpha_bandwidth;
        feedback_1 = -2 * cos_w0;
        feedback_2 = 1 - alpha_bandwidth;
        break;
    case Type::Notch:
        input_0 = 1;
        input_1 = -2 * cos_w0;
        input_2 = 1;

        feedback_0 = 1 + alpha_bandwidth;
        feedback_1 = -2 * cos_w0;
        feedback_2 = 1 - alpha_bandwidth;
        break;
    default:
        assert(false);
        Util::should_not_be_reached();
    }

    feedback_1 /= feedback_0;
    feedback_2 /= feedback_0;
    input_0 /= feedback_0;
    input_1 /= feedback_0;
    input_2 /= feedback_0;
    feedback_0 = 1;
}

float Coefficients::response_at(Hertz frequency) const {
    float phi = sinf(Util::two_pi_f * frequency / (2 * m_sample_rate));
    phi *= phi;
    float insum = input_0 + input_1 + input_2;
    float feedback_sum = feedback_0 + feedback_1 + feedback_2;
    float fsqr = (16 * input_0 * input_2 * phi * phi + insum * insum - 4 * (input_0 * input_1 + 4 * input_0 * input_2 + input_1 * input_2) * phi) /
                 (16 * feedback_2 * phi * phi + feedback_sum * feedback_sum - 4 * (feedback_1 * feedback_2 + feedback_1 + 4 * feedback_2) * phi);
    return fsqr < 0 ? 0 : std::sqrt(fsqr);
}

}
