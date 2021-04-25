#pragma once

#include "ProcessingThread.hpp"
#include <alsa/asoundlib.h>

namespace Dsp {

class SoundCardThread final : public ProcessingThread {
public:
    SoundCardThread(std::shared_ptr<Dsp::DecoderBase> decoder, std::string input_name, SampleRate sample_rate);
    virtual ~SoundCardThread() override;
    bool init_soundcard();

private:
    virtual size_t fill_buffer(Util::Buffer<float>&) override;

    std::string m_input_name;
    SampleRate m_sample_rate;
    snd_pcm_t* m_snd_handle { nullptr };
};

}
