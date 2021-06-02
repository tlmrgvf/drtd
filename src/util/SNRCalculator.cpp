#include "SNRCalculator.hpp"
#include <algorithm>
#include <cmath>
#include <util/Util.hpp>

using namespace Util;

SNRCalculator::SNRCalculator(Samples window_size)
    : m_window_size(window_size) {
}

void SNRCalculator::collect_signal_and_noise_sample(float power) {
    if (m_enabled)
        m_signal_noise_sum += power;
}

void SNRCalculator::collect_signal_sample(float power) {
    if (m_enabled)
        m_signal_sum += power;
}

bool SNRCalculator::commit_samples() {
    if (!m_enabled)
        return false;

    ++m_sample_count;
    if (m_sample_count >= m_window_size) {
        m_snr_db = Util::db_power(m_signal_sum, m_signal_noise_sum - m_signal_sum);
        m_sample_count = 0;
        m_signal_sum = 0;
        m_signal_noise_sum = 0;
        return true;
    }

    return false;
}
