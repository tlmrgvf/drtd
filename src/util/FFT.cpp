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
