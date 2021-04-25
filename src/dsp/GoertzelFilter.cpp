#include "GoertzelFilter.hpp"
#include <Fl/fl_draw.h>
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
