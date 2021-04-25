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
#include "Scope.hpp"
#include <Fl/Fl.H>
#include <ui/ScopeDialog.hpp>
#include <util/Point.hpp>

using namespace Ui;

void Scope::recalculate_bias() {
    m_dc_bias = m_sample_min + (m_sample_max - m_sample_min) / 2.f;
}

int Scope::handle(int event) {
    if (event == FL_PUSH && Fl::event_button3())
        ScopeDialog::show_dialog();

    return Canvas::handle(event);
}

void Scope::process_sample(float sample) {
    if (m_paused && !m_single_shot)
        return;

    bool oversampling = m_settings.zoom > 0;
    auto abs_zoom = std::abs(m_settings.zoom);
    auto layer_width = m_layer->current_width();
    auto layer_height = m_layer->current_height();

    if (layer_width < 10 || layer_height < 10)
        return;

    if ((!oversampling && m_samples_captured >= m_width) || (oversampling && m_samples_captured * abs_zoom >= m_width)) {
        if (m_limiter.limit() && m_samples.size()) {
            recalculate_bias();
            const float bias = real_bias();
            float adj_max;
            u32 half_height = m_layer->current_height() / 2;

            if (m_settings.normalized) {
                adj_max = 1;
            } else {
                adj_max = std::max(m_sample_max, std::abs(m_sample_min)) - bias;
                adj_max *= 1.1f;
            }

            {
                Ui::LayerDraw draw(m_layer);

                fl_rectf(0, 0, layer_width, layer_height, m_layer->clear_color());
                fl_color(FL_GRAY);
                fl_line(0, half_height, layer_width - 1, half_height);

                fl_color(FL_RED);
                if (adj_max > 0) {
                    unsigned x = 0;
                    Util::Point last_point;
                    bool first = true;

                    for (unsigned i = 0; i < m_width; ++i) {
                        float buffer_sample = m_samples[i];
                        Util::Point p(x, static_cast<int>((((buffer_sample - bias) / adj_max) * -static_cast<float>(half_height) + static_cast<float>(half_height))));
                        if (first) {
                            last_point = p;
                            first = false;
                            continue;
                        }

                        fl_line(last_point.x(), last_point.y(), last_point.x(), p.y(), p.x(), p.y());

                        if ((last_point.x() + abs_zoom) >= static_cast<int>(m_width) && oversampling)
                            break;

                        last_point = p;
                        ++x;
                        if (oversampling)
                            x += abs_zoom;
                    }
                }
            }

            damage(FL_DAMAGE_ALL);
        }

        m_samples_captured = 0;
        m_single_shot = false;
    }

    if (m_samples_captured == 0) {
        recalculate_bias();
        m_sample_max = std::max(m_sample_max, sample);
        m_sample_min = std::min(m_sample_min, sample);
        if (layer_width != m_width) {
            m_width = layer_width;
            m_samples = Util::Buffer<float>(m_width);
        }

        m_sample_max = std::numeric_limits<float>::min();
        m_sample_min = std::numeric_limits<float>::max();
    }

    if (abs_zoom == 0 || oversampling) {
        m_samples[m_samples_captured++] = sample;
        m_sample_max = std::max(m_sample_max, sample);
        m_sample_min = std::min(m_sample_min, sample);
    } else if (m_samples_averaged > static_cast<u32>(abs_zoom)) {
        float avgSample = m_sample_average / static_cast<float>(abs_zoom + 1);
        m_samples[m_samples_captured++] = avgSample;
        m_sample_max = std::max(m_sample_max, avgSample);
        m_sample_min = std::min(m_sample_min, avgSample);
        m_samples_averaged = 0;
        m_sample_average = 0;
    } else {
        ++m_samples_averaged;
        m_sample_average += sample;
    }

    m_last_sample = sample;
}
