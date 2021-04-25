#pragma once

#include "ProcessingThread.hpp"
#include <csignal>
#include <util/Resampler.hpp>

namespace Dsp {

class StdinThread final : public ProcessingThread {
public:
    enum class SampleSize : bool {
        S8,
        S16
    };

    StdinThread(std::shared_ptr<Dsp::DecoderBase> decoder, SampleRate input_sample_rate, SampleRate target_sample_rate, bool big_endian, SampleSize sample_size);

private:
    virtual size_t fill_buffer(Util::Buffer<float>&) override;
    virtual void on_stop_requested() override;

    SampleSize m_sample_size;
    Util::Buffer<int8_t> m_buffer;
    bool m_byte_index { false };
    int8_t m_older_sample { 0 };
    int8_t m_newer_sample { 0 };
    int8_t& m_bigger;
    int8_t& m_smaller;
    std::unique_ptr<struct sigaction> m_sigaction;
};

}
