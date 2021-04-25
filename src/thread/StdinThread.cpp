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
#include "StdinThread.hpp"

#include <unistd.h>

using namespace Dsp;

static const Util::Logger s_log("StdinThread");

StdinThread::StdinThread(std::shared_ptr<Dsp::DecoderBase> decoder, SampleRate input_sample_rate, SampleRate target_sample_rate, bool big_endian, SampleSize sample_size)
    : ProcessingThread(decoder, input_sample_rate, target_sample_rate)
    , m_sample_size(sample_size)
    , m_buffer(s_sample_buffer_size * (sample_size == SampleSize::S16 ? 2 : 1))
    , m_bigger(big_endian ? m_older_sample : m_newer_sample)
    , m_smaller(big_endian ? m_newer_sample : m_older_sample) {
    s_log.info() << "Started thread for input S/R " << input_sample_rate
                 << ", output S/R " << target_sample_rate
                 << ", big endian: " << big_endian
                 << ", sample size of " << (sample_size == SampleSize::S16 ? "16 bits" : "8 bits");

    m_sigaction = std::make_unique<struct sigaction>();
    memset(m_sigaction.get(), 0, sizeof(struct sigaction));
    m_sigaction->sa_handler = [](int) {};
    sigaction(SIGUSR1, m_sigaction.get(), nullptr);
}

void StdinThread::on_stop_requested() {
    s_log.info() << "Stopping thread";
    pthread_kill(thread().native_handle(), SIGUSR1);
}

size_t StdinThread::fill_buffer(Util::Buffer<float>& output_buffer) {
    auto read_bytes = read(STDIN_FILENO, static_cast<void*>(m_buffer.ptr()), m_buffer.size());
    if (read_bytes < 0) {
        if (errno == EINTR) {
            s_log.info() << "read() interrupted";
            request_stop();
        } else {
            s_log.error() << "Error reading from stdin: " << strerror(errno);
        }

        return 0;
    }

    if (m_sample_size == SampleSize::S8) {
        for (long i = 0; i < read_bytes; ++i)
            output_buffer[i] = m_buffer[i] / static_cast<float>(std::numeric_limits<int16_t>::max());
    } else if (m_sample_size == SampleSize::S16) {
        size_t output_index = 0;
        for (long i = 0; i < read_bytes; ++i) {
            if (m_byte_index) {
                m_newer_sample = m_buffer[i];
                i16 sample = static_cast<i16>((m_bigger << 8) | (m_smaller & 0xFF));
                output_buffer[output_index++] = sample / static_cast<float>(std::numeric_limits<int16_t>::max());
            } else {
                m_older_sample = m_buffer[i];
            }

            m_byte_index = !m_byte_index;
        }
        read_bytes = output_index;
    }

    return read_bytes;
}
