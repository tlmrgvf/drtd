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
#include "NRZIDecoder.hpp"
#include <FL/fl_draw.H>

static constexpr const char* nrzidecoder_xpm[] {
    "30 26 2 1",
    " 	c None",
    ".	c #000000",
    "..............................",
    ".                            .",
    ".                            .",
    ".                            .",
    ".                            .",
    ".    ....  ....              .",
    ".    .  .  .  .              .",
    ".    .  .  .  .              .",
    ".    .  .  .  .              .",
    ".    .  .  .  .              .",
    ".    .  .  .  .              .",
    ".    .  ....  ...........    .",
    ".                            .",
    ".                            .",
    ".                            .",
    ".    ..........              .",
    ".             .              .",
    ".             .              .",
    ".             .              .",
    ".             .              .",
    ".             .              .",
    ".             ...........    .",
    ".                            .",
    ".                            .",
    ".                            .",
    ".............................."
};

using namespace Dsp;

NRZIDecoder::NRZIDecoder(bool inverted)
    : ComponentBase<bool, bool>("NRZI Decoder")
    , m_inverted(inverted) {
}

Size NRZIDecoder::calculate_size() {
    int width = 0;
    int height = 0;
    fl_measure_pixmap(nrzidecoder_xpm, width, height);
    return { static_cast<unsigned>(width), static_cast<unsigned>(height) };
}

void NRZIDecoder::draw_at(Point p) {
    fl_draw_pixmap(nrzidecoder_xpm, p.x(), p.y());
}

bool NRZIDecoder::process(bool sample) {
    bool ret = (sample == m_last_sample) == m_inverted;
    m_last_sample = sample;
    return ret;
}
