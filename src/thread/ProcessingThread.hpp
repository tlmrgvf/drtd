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
