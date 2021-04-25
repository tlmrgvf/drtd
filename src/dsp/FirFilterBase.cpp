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
