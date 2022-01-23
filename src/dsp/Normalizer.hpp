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

#include <FL/fl_draw.H>
#include <cmath>
#include <limits>
#include <pipe/Component.hpp>
#include <util/RingBuffer.hpp>
#include <util/Types.hpp>

namespace Dsp {

class Normalizer final : public RefableComponent<float, float, Normalizer> {
public:
    enum class Lookahead : bool {
        Yes,
        No
    };

    enum class OffsetMode : bool {
        Minimum,
        Average
    };

    Normalizer(WindowSize, Lookahead, OffsetMode);

    virtual Size calculate_size() override;

    void set_window_size(WindowSize);
    WindowSize window_size() const { return m_window_size; }

protected:
    virtual Normalizer& ref() override { return *this; }
    virtual void draw_at(Point) override;
    virtual float process(float) override;

private:
    WindowSize m_window_size;
    Lookahead m_lookahead;
    OffsetMode m_offset_mode;
    RingBuffer<float> m_delay_buffer;
    float m_factor { 1 };
    float m_average { 0 };
    float m_offset { 0 };
    float m_count { 0 };
    float m_max { std::numeric_limits<float>::min() };
    float m_min { std::numeric_limits<float>::max() };
};

}
