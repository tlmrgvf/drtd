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
