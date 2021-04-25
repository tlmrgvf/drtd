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
#include <atomic>
#include <dsp/Window.hpp>
#include <util/FFT.hpp>
#include <util/Limiter.hpp>
#include <util/Marker.hpp>
#include <util/RingBuffer.hpp>
#include <util/Size.hpp>
#include <util/Types.hpp>

namespace Dsp {
class DecoderBase;
}

namespace Ui {

class Waterfall final : public Canvas {
public:
    struct Settings {
        u32 bins { 4096 };
        u8 speed_multiplier { 0 };
        bool power_spectrum { false };
        float zoom { 0 };
        u32 bin_offset { 0 };
    };

    struct GlobalSettings {
        u8 palette_index { 0 };
        u8 window_index { 0 };
    };

    Waterfall(GlobalSettings settings, u32 x, u32 y, u32 w, u32 h);

    static u32 pseudo_bins(const Settings& settings) {
        return static_cast<u32>(std::roundf((down_sampling(settings) ? 1 / (std::abs(settings.zoom) + 1) : settings.zoom + 1) * settings.bins));
    }

    static float hz_per_bin(const Settings& settings, SampleRate sample_rate) { return sample_rate / static_cast<float>(pseudo_bins(settings)); }
    static i32 translate_hz_to_x(const Settings& settings, i32 hz, SampleRate sample_rate) {
        return static_cast<i32>(std::roundf(static_cast<float>(hz) / hz_per_bin(settings, sample_rate)));
    }

    static float translate_x_to_hz(const Settings& settings, u32 x, SampleRate sample_rate) { return static_cast<float>(x) * hz_per_bin(settings, sample_rate); }

    void set_palette_index(u8 index) { m_global_settings.palette_index = index; }
    void set_window_index(u8 index) {
        m_new_window_index = index;
        m_update_settings = true;
    }

    u8 palette_index() const { return m_global_settings.palette_index; };
    u8 window_index() const { return m_new_window_index; };
    const GlobalSettings& global_settings() const { return m_global_settings; }
    const Settings& settings() const { return m_settings; }
    const Settings& new_settings() const { return m_new_settings; }
    void update_settings_later(const Settings& settings);
    void process_sample(float sample);
    void set_sample_rate(SampleRate sample_rate);
    SampleRate sample_rate() const { return m_sample_rate; }
    void set_decoder(std::shared_ptr<Dsp::DecoderBase>&);
    void show_marker(bool show);
    void force_redraw();

private:
    static constexpr bool down_sampling(const Settings& settings) { return settings.zoom < 0; }

    virtual int handle(int event) override;
    void update_settings(bool force);
    void redraw_scale_and_marker();
    void redraw_scale_later();

    u8 m_new_window_index;
    Settings m_new_settings;
    Settings m_settings;
    GlobalSettings m_global_settings;
    std::shared_ptr<Layer> m_waterfall_layer;
    std::shared_ptr<Layer> m_scale_layer;
    Util::RingBuffer<float> m_samples;
    Util::Buffer<float> m_window_coefficients;
    u32 m_sample_count { 0 };
    int m_old_width { 0 };
    SampleRate m_sample_rate { 44100 };
    std::mutex m_settings_mutex;
    bool m_update_settings { false };
    bool m_redraw_scale { false };
    Limiter m_redraw_limiter { 60 };
    Limiter m_input_limiter { 30 };
    std::shared_ptr<Dsp::DecoderBase> m_decoder;
    FFT m_fft;
    bool m_show_marker { true };
    Size m_old_size;
};

namespace Palette {
struct WaterfallPalette {
    const char* name { "" };
    const Fl_Color* colors { nullptr };
    size_t color_count { 0 };
};

const Util::Buffer<WaterfallPalette>& palettes();
}
}
