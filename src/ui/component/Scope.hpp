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
#include <FL/Fl.H>
#include <limits>
#include <util/Buffer.hpp>
#include <util/Limiter.hpp>

namespace Ui {

class Scope final : public Canvas {
public:
    struct Settings {
        i8 zoom { 0 };
        bool remove_dc_bias { true };
        bool normalized { false };
    };

    Scope(int x, int y, int w, int h)
        : Canvas(x, y, w, h, 2)
        , m_layer(make_layer(0, 0, Layer::parent_size, Layer::parent_size)) {
        box(FL_DOWN_BOX);
    }

    void process_sample(float sample);
    Settings& settings() { return m_settings; }
    void single_shot() { m_single_shot = true; }
    void set_paused(bool paused) { m_paused = paused; }

private:
    virtual int handle(int event) override;
    void recalculate_bias();
    float real_bias() { return m_settings.remove_dc_bias ? m_dc_bias : 0; }

    std::shared_ptr<Layer> m_layer;
    Settings m_settings;
    Util::Limiter m_limiter { 30 };
    Util::Buffer<float> m_samples;
    u32 m_width { 1 };
    u32 m_samples_captured { 0 };
    u32 m_samples_averaged { 0 };
    float m_sample_average { 0 };
    float m_sample_max { std::numeric_limits<float>::min() };
    float m_sample_min { std::numeric_limits<float>::max() };
    float m_dc_bias { 0 };
    float m_last_sample { 0 };
    bool m_single_shot { false };
    bool m_paused { false };
};
}
