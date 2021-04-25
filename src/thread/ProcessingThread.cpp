#include "ProcessingThread.hpp"

using namespace Dsp;

ProcessingLock::ProcessingLock(bool from_main_thread) {
    if (Drtd::using_ui() && from_main_thread)
        Fl::unlock();

    s_pipeline_mutex.lock();

    if (Drtd::using_ui() && from_main_thread)
        Fl::lock();
}

ProcessingLock::~ProcessingLock() {
    s_pipeline_mutex.unlock();
}

ProcessingThread::ProcessingThread(std::shared_ptr<Dsp::DecoderBase> decoder, SampleRate input_sample_rate, SampleRate target_sample_rate)
    : m_decoder(decoder) {
    if (input_sample_rate != target_sample_rate)
        m_resampler = std::make_unique<Util::Resampler>(input_sample_rate, target_sample_rate);
}

void ProcessingThread::start() {
    m_thread = std::thread(&ProcessingThread::run, this);
    m_running = true;
}

bool ProcessingThread::is_running() {
    return m_running;
}

void ProcessingThread::request_stop_and_wait() {
    m_run.store(false);
    on_stop_requested();
    if (Drtd::using_ui())
        Fl::unlock();

    join();

    if (Drtd::using_ui())
        Fl::lock();
    m_running = false;
}

void ProcessingThread::join() {
    m_thread.join();
}

void ProcessingThread::run() {
    Buffer<float> buffer(s_sample_buffer_size);
    size_t read = 0;
    float sample = 0;

    if (m_resampler) {
        while (m_run.load()) {
            if ((read = fill_buffer(buffer))) {
                assert(read <= s_sample_buffer_size);
                ProcessingLock lock(false); //Lock components
                Fl::lock();                 //Lock fltk

                for (size_t i = 0; i < read && m_run.load(); ++i) {
                    m_resampler->process_input_sample(buffer[i]);
                    while (m_resampler->read_output_sample(sample))
                        m_decoder->process(buffer[i]);
                }

                Fl::awake();
                Fl::unlock();
            } else if (m_run.load()) {
                Util::die("Error reading samples!");
            }
        }
    } else {
        while (m_run.load()) {
            if ((read = fill_buffer(buffer))) {
                assert(read <= s_sample_buffer_size);
                ProcessingLock lock(false); //Lock components
                Fl::lock();                 //Lock fltk

                for (size_t i = 0; i < read && m_run.load(); ++i)
                    m_decoder->process(buffer[i]);

                Fl::awake();
                Fl::unlock();
            } else if (m_run.load()) {
                Util::die("Error reading samples!");
            }
        }
    }
}
