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
