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
