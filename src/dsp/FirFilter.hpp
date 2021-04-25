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

#include <dsp/Window.hpp>
#include <pipe/Component.hpp>
#include <ui/FirFilterDialog.hpp>
#include <util/RingBuffer.hpp>
#include <util/Types.hpp>

namespace Dsp {

struct FirFilterProperties {
    WindowType window_type { WindowType::Rectangular };
    Taps taps { 1 };
    Hertz start_frequency { 0 };
    Hertz stop_frequency { 0 };
    bool band_stop { false };
};

class FirFilterBase {
public:
    FirFilterBase(FirFilterProperties properties);
    virtual ~FirFilterBase() = default;

    void set_properties(FirFilterProperties);

    Taps taps() const { return m_properties.taps; }
    Hertz start_frequency() const { return m_properties.start_frequency; }
    Hertz stop_frequency() const { return m_properties.stop_frequency; }
    bool is_band_stop() const { return m_properties.band_stop; }
    WindowType window_type() const { return m_properties.window_type; }
    SampleRate sample_rate() const { return m_sample_rate; }
    const Buffer<float>& coefficients() const { return m_coefficients; }
    FirFilterProperties properties() const { return m_properties; }

protected:
    void recalculate_coefficients();
    virtual void on_recalculate() = 0;
    Buffer<float> m_coefficients;

    SampleRate m_sample_rate { 0 };
    FirFilterProperties m_properties;
};

template<typename T>
class FirFilter final : public RefableComponent<T, T, FirFilterBase>
    , public FirFilterBase {
public:
    FirFilter(WindowType window, Taps taps, Hertz freq_start, Hertz freq_stop, bool band_stop = false)
        : RefableComponent<T, T, FirFilterBase>("Fir Filter")
        , FirFilterBase({ window, taps, freq_start, freq_stop, band_stop })
        , m_sample_buffer(taps) {
    }

    virtual Size calculate_size() override {
        return size;
    }

protected:
    virtual void on_recalculate() override {
        m_sample_buffer.resize(m_properties.taps);
    }

    virtual FirFilterBase& ref() override {
        return *this;
    }

    virtual SampleRate on_init(SampleRate input_sample_rate, int&) override {
        m_sample_rate = input_sample_rate;
        recalculate_coefficients();
        return input_sample_rate;
    }

    static void draw_sine(const Point& p, bool strikethrough, u8 index) {
        constexpr auto width = size.w() * .7f;
        constexpr auto height = size.h() * .7f;
        constexpr float sin_height = height / 6;
        const auto y_offset = static_cast<float>(p.y() + Util::center(size.h(), static_cast<unsigned>(height))) + 2 * sin_height * index + sin_height;
        const auto x_offset = p.x() + Util::center(size.w(), static_cast<unsigned>(width));

        fl_begin_line();
        fl_vertex(x_offset, y_offset);
        for (float xi = 1; xi < width; ++xi) {
            float yi = sin_height * sinf(2 * static_cast<float>(M_PI) * (xi / width)) + y_offset;
            fl_vertex(xi + static_cast<float>(x_offset), yi);
        }
        fl_end_line();

        if (strikethrough)
            fl_line(p.x() + size.w() / 2 - 2, static_cast<int>(y_offset) - 2, p.x() + size.w() / 2 + 2, static_cast<int>(y_offset) + 2);
    }

    virtual void draw_at(Point p) override {
        unsigned mid_range_lo = this->input_sample_rate() / 6;
        unsigned mid_range_hi = this->input_sample_rate() / 3;
        bool remove_low = (start_frequency() > mid_range_lo) ^ is_band_stop();
        bool remove_mid = (start_frequency() > mid_range_hi || stop_frequency() < mid_range_lo) ^ is_band_stop();
        bool remove_high = (stop_frequency() < mid_range_hi) ^ is_band_stop();

        fl_rect(p.x(), p.y(), size.w(), size.h());
        draw_sine(p, remove_low, 0);
        draw_sine(p, remove_mid, 1);
        draw_sine(p, remove_high, 2);
    }

    virtual void show_config_dialog() override {
        Ui::FirFilterDialog::show_dialog(this->make_ref());
    }

    virtual T process(T sample) override {
        if (taps() == 1)
            return sample;

        m_sample_buffer.push(sample);
        T sum {};
        for (Taps tap = 0; tap < taps(); ++tap)
            sum += m_sample_buffer.peek(tap) * m_coefficients[tap];

        return sum;
    }

private:
    static constexpr Size size { 60, 45 };

    RingBuffer<T> m_sample_buffer;
};

}
