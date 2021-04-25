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

#include <Fl/fl_draw.H>
#include <pipe/Component.hpp>
#include <ui/MovingAverageDialog.hpp>
#include <util/RingBuffer.hpp>
#include <util/Types.hpp>

namespace Dsp {

class MovingAverageBase {
public:
    virtual ~MovingAverageBase() = default;

    virtual void set_taps(Taps taps) = 0;
    virtual Taps taps() const = 0;
    SampleRate sample_rate() const { return m_sample_rate; }

protected:
    SampleRate m_sample_rate { 0 };
};

template<typename T>
class MovingAverage final : public RefableComponent<T, T, MovingAverageBase>
    , public MovingAverageBase {
public:
    MovingAverage(Taps taps)
        : RefableComponent<T, T, MovingAverageBase>("Moving average")
        , m_buffer(taps)
        , m_taps(taps) {
    }

    virtual Size calculate_size() override {
        int width = 0;
        int height = 0;

        fl_measure_pixmap(moving_average_xpm, width, height);
        return { static_cast<unsigned>(width), static_cast<unsigned>(height) };
    }

    virtual void set_taps(Taps taps) override {
        if (taps == m_taps)
            return;

        m_taps = taps;
        m_buffer.resize(m_taps);
        m_average = 0;
        m_zero_count = 0;
        Ui::MovingAverageDialog::update_dialog();
    }

    virtual Taps taps() const override {
        return m_taps;
    }

protected:
    virtual SampleRate on_init(SampleRate input_sample_rate, int&) override {
        m_sample_rate = input_sample_rate;
        return input_sample_rate;
    }

    virtual MovingAverageBase& ref() override {
        return *this;
    }

    virtual void draw_at(Point p) override {
        fl_draw_pixmap(moving_average_xpm, p.x(), p.y());
    }

    virtual void show_config_dialog() override {
        Ui::MovingAverageDialog::show_dialog(this->make_ref());
    }

    virtual T process(T sample) override {
        if (m_taps < 2)
            return sample;

        m_average = (m_average + sample - m_buffer.push(sample));

        if (sample == 0) {
            if (m_zero_count > m_taps)
                m_average = 0;
            else
                ++m_zero_count;
        } else {
            m_zero_count = 0;
        }

        return m_average / m_taps;
    }

private:
    static constexpr const char* moving_average_xpm[] = {
        "21 14 2 1",
        " 	c None",
        ".	c #000000",
        ".....................",
        ".                   .",
        ".     .........     .",
        ".     .       .     .",
        ".     .       .     .",
        ".     .       .     .",
        ".     .       .     .",
        ". .....   .   ..... .",
        ".        . .        .",
        ".       .   .       .",
        ".      .     .      .",
        ". .....       ..... .",
        ".                   .",
        "....................."
    };

    RingBuffer<T> m_buffer;
    T m_average {};
    Taps m_taps { 1 };
    size_t m_zero_count { 0 };
};

}
