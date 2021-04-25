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
#include "GoertzelFilter.hpp"
#include <Fl/fl_draw.H>
#include <cmath>

using namespace Dsp;

static constexpr const char* goertzelfilter_xpm[] {
    "21 12 2 1",
    " 	c None",
    ".	c #000000",
    ".....................",
    ".                   .",
    ".                   .",
    ".         .         .",
    ".        ...        .",
    ".        . .        .",
    ".       .. ..       .",
    ".       .   .       .",
    ".      ..   ..      .",
    ". ......     ...... .",
    ".                   .",
    "....................."
};

GoertzelFilter::GoertzelFilter(Taps taps, float frequency)
    : ComponentBase<float, float>("Goertzel filter")
    , m_taps(taps)
    , m_frequency(frequency)
    , m_buffer(taps) {
}

SampleRate GoertzelFilter::on_init(SampleRate input_sample_rate, int&) {
    m_coefficient = 2 * cosf(Util::two_pi_f / m_taps * std::roundf(m_taps / static_cast<float>(input_sample_rate) * m_frequency));
    return input_sample_rate;
}

float GoertzelFilter::process(float sample) {
    m_buffer.push(sample);

    float v1 = 0;
    float v2 = 0;
    for (Taps i = 0; i < m_taps; ++i) {
        const float value = m_coefficient * v1 - v2 + m_buffer.peek(i);
        v2 = v1;
        v1 = value;
    }

    return sqrtf(v2 * v2 + v1 * v1 - m_coefficient * v1 * v2);
}

Size GoertzelFilter::calculate_size() {
    int width = 0;
    int height = 0;
    fl_measure_pixmap(goertzelfilter_xpm, width, height);
    return { static_cast<unsigned>(width), static_cast<unsigned>(height) };
}

void GoertzelFilter::draw_at(Point p) {
    fl_draw_pixmap(goertzelfilter_xpm, p.x(), p.y());
}
