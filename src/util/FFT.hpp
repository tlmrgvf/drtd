#pragma once

#include "Buffer.hpp"
#include "fftw3.h"
#include <optional>

namespace Util {

class FFT {
public:
    FFT()
        : FFT(0) {
    }

    explicit FFT(size_t bins)
        : FFT(Buffer<double>(bins)) {
    }
    explicit FFT(Buffer<double> input_buffer);
    FFT(FFT&) = delete;
    FFT(FFT&& move);
    ~FFT();

    Buffer<double>& input_buffer() { return m_fft_in; }
    void set_input_buffer(Buffer<double> buffer) { m_fft_in = buffer; }
    Buffer<fftw_complex>& output_buffer() { return m_fft_out; }
    void execute();
    FFT& operator=(FFT&& to_move);

private:
    friend void swap(FFT& one, FFT& two) {
        std::swap(one.m_fft_in, two.m_fft_in);
        std::swap(one.m_fft_out, two.m_fft_out);
        std::swap(one.m_fft_plan, two.m_fft_plan);
        std::swap(one.m_valid, two.m_valid);
    }

    Buffer<double> m_fft_in;
    Buffer<fftw_complex> m_fft_out;
    fftw_plan m_fft_plan;
    bool m_valid { false };
};

}

using Util::FFT;
