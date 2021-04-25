#include "Normalizer.hpp"

using namespace Dsp;

static constexpr const char* normalizer_xpm[] {
    "22 22 2 1",
    " 	c None",
    ".	c #000000",
    "......................",
    ".                    .",
    ".     .............. .",
    ".     .              .",
    ".     .   .          .",
    ".     .   .          .",
    ".     .  ...         .",
    ".     .  . .         .",
    ".  .  . .. ..        .",
    ". ..  . .   .        .",
    ".  . .. .   .        .",
    ".  .  .     ..   .   .",
    ". ... .      .   .   .",
    ".     .      .. ..   .",
    ".     .       . .    .",
    ".     .       ...    .",
    ".     .        .     .",
    ".     .        .     .",
    ".     .              .",
    ".     .............. .",
    ".                    .",
    "......................"
};

Normalizer::Normalizer(WindowSize window_size, Lookahead lookahead, OffsetMode offset_mode)
    : RefableComponent<float, float, Normalizer>("Normalizer")
    , m_window_size(window_size)
    , m_lookahead(lookahead)
    , m_offset_mode(offset_mode) {
    assert(window_size);
    if (lookahead == Lookahead::Yes)
        m_delay_buffer.resize(window_size);
}

Size Normalizer::calculate_size() {
    int width = 0;
    int height = 0;

    fl_measure_pixmap(normalizer_xpm, width, height);
    return { static_cast<unsigned>(width), static_cast<unsigned>(height) };
}

void Normalizer::draw_at(Point p) {
    fl_draw_pixmap(normalizer_xpm, p.x(), p.y());
}

float Dsp::Normalizer::process(float sample) {
    float normalized = sample;
    if (m_lookahead == Lookahead::Yes)
        sample = m_delay_buffer.push(sample);

    normalized = (normalized - m_offset) * m_factor;
    if (m_count >= m_window_size) {
        if (m_offset_mode == OffsetMode::Average) {
            m_offset = m_average / m_count;
            m_factor = 1 / (m_offset - m_min);
        } else {
            m_offset = m_min;
            m_factor = 1 / (m_max - m_min);
        }

        m_average = 0;
        m_count = 0;
        m_max = std::numeric_limits<float>::min();
        m_min = std::numeric_limits<float>::max();
    } else {
        ++m_count;

        if (m_offset_mode == OffsetMode::Average)
            m_average += sample;

        m_max = std::max(m_max, sample);
        m_min = std::min(m_min, sample);
    }

    return std::isnan(normalized) ? 0 : normalized;
}

void Normalizer::set_window_size(WindowSize window_size) {
    assert(window_size);
    m_window_size = window_size;
    if (m_lookahead == Lookahead::Yes)
        m_delay_buffer.resize(window_size);

    m_factor = 1;
    m_average = 0;
    m_count = 0;
    m_max = std::numeric_limits<float>::min();
    m_min = std::numeric_limits<float>::max();
}
