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
#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <string>
#include <util/Types.hpp>

namespace Dsp::Biquad {

static constexpr float invsqrt2 { static_cast<float>(M_SQRT1_2) }; /* 1 / sqrt(2) */

enum class Type : u8 {
    Lowpass,
    Highpass,
    BandpassSkirt,
    BandpassPeak,
    Notch,
    __Count
};

struct Coefficients final {
    using StringArray = std::array<const char*, static_cast<size_t>(Type::__Count)>;
    static constexpr StringArray names { "Lowpass", "Highpass", "Bandpass (Constant skirt gain)", "Bandpass (Constant peak gain)", "Notch" };
    static constexpr StringArray parameter_descriptions { "Q", "Q", "Width (octaves)", "Width (octaves)", "Bandwidth (octaves)" };

    static std::string name_for(Type);
    static std::string parameter_description_for(Type);

    Coefficients() = default;
    Coefficients(Type type, SampleRate sample_rate, float center, float q);
    float input_0 { 0 };
    float input_1 { 0 };
    float input_2 { 0 };

    float feedback_0 { 0 };
    float feedback_1 { 0 };
    float feedback_2 { 0 };

    SampleRate m_sample_rate;
    float response_at(Hertz frequency) const;
};

template<typename T>
class Filter final {
public:
    Filter() = default;
    Filter(Type type, SampleRate sample_rate, float center, float q)
        : m_type(type)
        , m_sample_rate(sample_rate)
        , m_center(center)
        , m_parameter(q) {
        recalculate_coefficients();
    }

    T filter_sample(T& input) {
        T result = input * m_coefficients.input_0 + m_z1;
        m_z1 = (input * m_coefficients.input_1 + m_z2) - result * m_coefficients.feedback_1;
        m_z2 = input * m_coefficients.input_2 - result * m_coefficients.feedback_2;
        return result;
    }

    void recalculate_coefficients() {
        m_coefficients = Coefficients(m_type, m_sample_rate, m_center, m_parameter);
    }

    float parameter() const { return m_parameter; }
    float center() const { return m_center; }
    Type type() const { return m_type; }
    SampleRate sample_rate() const { return m_sample_rate; }

    void set_parameter(float parameter) { m_parameter = parameter; }
    void set_center(float center) { m_center = center; }
    void set_type(Type type) { m_type = type; }
    void set_sample_rate(SampleRate sample_rate) { m_sample_rate = sample_rate; }

private:
    Type m_type { Type::Lowpass };
    SampleRate m_sample_rate { 1 };
    Coefficients m_coefficients;
    float m_center { 0 };
    float m_parameter { 0 };
    T m_z1 {};
    T m_z2 {};
};
}
