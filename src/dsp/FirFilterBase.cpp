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
#include "Drtd.hpp"
#include "FirFilter.hpp"
#include <cstdio>

using namespace Dsp;

FirFilterBase::FirFilterBase(FirFilterProperties properties)
    : m_properties(properties) {
    assert(m_properties.start_frequency <= m_properties.stop_frequency);
    assert(m_properties.taps);

    if (m_properties.taps % 2 == 0)
        ++m_properties.taps;
}

void FirFilterBase::set_properties(FirFilterProperties properties) {
    m_properties = properties;
    recalculate_coefficients();
    Ui::FirFilterDialog::update_dialog();
}

void FirFilterBase::recalculate_coefficients() {
    assert(m_sample_rate);
    assert(m_properties.start_frequency <= m_properties.stop_frequency);
    assert(m_properties.taps);

    if (m_properties.taps % 2 == 0)
        ++m_properties.taps;

    m_coefficients = Buffer<float>(m_properties.taps);
    constexpr float pi_f = static_cast<float>(M_PI);

    Buffer<float> window_coefficients(m_properties.taps);
    Window::make(m_properties.window_type).calculate_coefficients(window_coefficients);
    const Taps center = static_cast<Taps>((m_properties.taps - 1) / 2);
    const float sample_rate = m_sample_rate;

    m_coefficients[center] = 2.f * (m_properties.stop_frequency - m_properties.start_frequency) / sample_rate;
    if (m_properties.band_stop)
        m_coefficients[center] = 1 - m_coefficients[center];

    const float value_sign = m_properties.band_stop ? -1 : +1;
    for (Taps i = center + 1; i < m_properties.taps; ++i) {
        const Taps norm = i - center;
        const float two_norm_pi = 2 * norm * pi_f;
        const Taps mirrored = static_cast<Taps>(m_properties.taps - i - 1);
        const float stop_term = sinf(two_norm_pi * (m_properties.stop_frequency / sample_rate));
        const float start_term = sinf(two_norm_pi * (m_properties.start_frequency / sample_rate));
        const float value = (stop_term - start_term) / (norm * pi_f) * window_coefficients[i] * value_sign;

        m_coefficients[i] = value;
        m_coefficients[mirrored] = value;
    }

    on_recalculate();
}
