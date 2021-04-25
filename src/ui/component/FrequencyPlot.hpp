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

#include "Canvas.hpp"
#include <util/Buffer.hpp>
#include <util/Size.hpp>

namespace Ui {

class FrequencyPlot final : public Canvas {
public:
    FrequencyPlot(u32 x, u32 y, u32 w, u32 h);

    void plot(Buffer<float> values, float max_frequency, float min_value, float max_value, float reference = 1);
    void plot();
    virtual int handle(int) override;

private:
    static constexpr unsigned db_scale_max = 200;
    static constexpr unsigned db_scale_min = 5;
    static constexpr unsigned db_scale_step = 5;
    static constexpr u8 frequency_divisions = 20;
    static constexpr u8 amplitude_divisions = 10;

    float m_max_frequency { 1 };
    Buffer<float> m_values;
    float m_min_value { 1 };
    float m_max_value { 1 };
    float m_reference { 1 };
    u32 m_vertical_scale { db_scale_max };
    std::shared_ptr<Layer> m_plot_layer;
    unsigned m_marker_x { 0 };
    bool m_plot_linear { false };
    bool m_show_marker { false };
    Size m_old_size;
};

}
