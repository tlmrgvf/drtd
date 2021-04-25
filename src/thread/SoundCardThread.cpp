#include "SoundCardThread.hpp"
#include <cmath>
#include <sstream>
#include <string>
#include <util/Logger.hpp>
#include <util/Util.hpp>

using namespace Dsp;

static const size_t s_real_buffer_size { ProcessingThread::s_sample_buffer_size * 2 };
static const Util::Logger s_log("SoundCardThread");
static u8 s_raw_sample_buffer[s_real_buffer_size];

SoundCardThread::SoundCardThread(std::shared_ptr<Dsp::DecoderBase> decoder, std::string input_name, uint16_t sample_rate)
    : ProcessingThread(decoder, sample_rate, sample_rate)
    , m_input_name(input_name)
    , m_sample_rate(sample_rate) {
}

SoundCardThread::~SoundCardThread() {
    if (!m_snd_handle) {
        s_log.warning() << "Attempted to close handle twice!";
        return;
    }

    s_log.info() << "Closing handle";
    snd_pcm_close(m_snd_handle);
    m_snd_handle = nullptr;
}

bool SoundCardThread::init_soundcard() {
    int err;
    if ((err = snd_pcm_open(&m_snd_handle, m_input_name.c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        s_log.warning() << "Failed to open sound device: Error calling snd_pcm_open: " << snd_strerror(err);
        return false;
    }

    s_log.info() << "Setting up sound device \"" << m_input_name << "\" with S/R of " << m_sample_rate;
    if ((err = snd_pcm_set_params(m_snd_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, m_sample_rate, 1, 100000)) < 0) {
        s_log.warning() << "Failed to set sound device parameters: Error calling snd_pcm_set_params: " << snd_strerror(err);
        return false;
    }

    return true;
}

size_t SoundCardThread::fill_buffer(Util::Buffer<float>& buffer) {
    assert(m_snd_handle);

    auto read = snd_pcm_readi(m_snd_handle, s_raw_sample_buffer, s_sample_buffer_size);
    if (read != s_sample_buffer_size) {
        s_log.error() << "Read unexpected ammount of samples!";
        return 0;
    }

    for (size_t i = 0; i + 1 < s_real_buffer_size; i += 2) {
        i16 sample = static_cast<i16>((s_raw_sample_buffer[i + 1] << 8) | (s_raw_sample_buffer[i] & 0xFF));
        buffer[i / 2] = sample / static_cast<float>(std::numeric_limits<int16_t>::max());
    }

    return read;
}
