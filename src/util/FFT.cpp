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
#include "FFT.hpp"

FFT::FFT(Buffer<double> input_buffer) {
    if (input_buffer.size()) {
        m_fft_in = input_buffer;
        m_fft_out = Buffer<fftw_complex>(input_buffer.size());
        m_fft_plan = fftw_plan_dft_r2c_1d(static_cast<int>(input_buffer.size()), m_fft_in.ptr(), m_fft_out.ptr(), FFTW_ESTIMATE);
        m_valid = true;
    }
}

FFT::FFT(FFT&& to_move) {
    swap(*this, to_move);
}

FFT::~FFT() {
    if (m_valid)
        fftw_destroy_plan(m_fft_plan);
}

void FFT::execute() {
    if (m_valid)
        fftw_execute(m_fft_plan);
}

FFT& FFT::operator=(FFT&& to_move) {
    swap(*this, to_move);
    return *this;
}
