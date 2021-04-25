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

#include <atomic>
#include <decoder/Decoder.hpp>
#include <thread>
#include <util/Buffer.hpp>
#include <util/Resampler.hpp>

namespace Dsp {

class ProcessingLock final {
public:
    ProcessingLock(bool from_main_thread = true);
    ~ProcessingLock();

private:
    ProcessingLock(const ProcessingLock&) = delete;
    ProcessingLock(const ProcessingLock&&) = delete;
    ProcessingLock& operator=(const ProcessingLock&) = delete;
    ProcessingLock& operator=(const ProcessingLock&&) = delete;

    static inline std::mutex s_pipeline_mutex;
};

class ProcessingThread {
public:
    static constexpr size_t s_sample_buffer_size { 1024 };

    ProcessingThread(std::shared_ptr<Dsp::DecoderBase> decoder, SampleRate input_sample_rate, SampleRate target_sample_rate);
    virtual ~ProcessingThread() {}

    void start();
    void request_stop_and_wait();
    void join();
    bool is_running();

protected:
    std::thread& thread() { return m_thread; }
    void request_stop() { m_run.store(false); };

private:
    void run();
    virtual void on_stop_requested() {};
    virtual size_t fill_buffer(Util::Buffer<float>&) = 0;

    std::unique_ptr<Util::Resampler> m_resampler;
    std::shared_ptr<Dsp::DecoderBase> m_decoder;
    std::atomic<bool> m_run { true };
    std::thread m_thread {};
    bool m_running { false };
};

}
