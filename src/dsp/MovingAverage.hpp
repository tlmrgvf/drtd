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
