#pragma once
#include <util/Types.hpp>

namespace Util {

class SNRCalculator final {
public:
    SNRCalculator(Samples window_size);

    void collect_signal_and_noise_sample(float power);
    void collect_signal_sample(float power);
    bool commit_samples();
    float snr_db() { return m_snr_db; }
    void set_enabled(bool enabled) { m_enabled = enabled; }

private:
    Samples m_window_size;
    Samples m_sample_count { 0 };
    float m_signal_noise_sum { 0 };
    float m_signal_sum { 0 };
    float m_snr_db { 0 };
    bool m_enabled { true };
};

}
