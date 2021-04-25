#include "XYScope.hpp"
#include <Fl/fl_draw.H>
#include <util/Point.hpp>

Ui::XYScope::XYScope(int x, int y, int size, WindowSize window_size, float fade_factor, bool connect)
    : Canvas(x, y, size, size, 2)
    , m_draw_layer(make_layer(0, 0, Ui::Layer::parent_size, Ui::Layer::parent_size))
    , m_fade_factor(fade_factor)
    , m_sample_buffer(window_size)
    , m_connect(connect) {
    box(FL_DOWN_BOX);
}

void Ui::XYScope::process(Util::Cmplx sample) {
    m_sample_buffer[m_sample_count++] = sample;
    m_max_magnitude = std::max(m_max_magnitude, sample.magnitude_squared());

    if (m_sample_count == m_sample_buffer.size()) {
        const auto width = m_draw_layer->current_width();
        const auto height = m_draw_layer->current_height();
        Buffer<u8> offscreen_buffer(width * height * 3);
        Ui::LayerDraw draw(m_draw_layer);

        fl_read_image(offscreen_buffer.ptr(), 0, 0, width, height);
        for (unsigned x = 0; x < width; ++x) {
            for (unsigned y = 0; y < height; ++y) {
                u8* rgb = offscreen_buffer.ptr() + (x + y * width) * 3;
                rgb[0] = static_cast<u8>(rgb[0] * m_fade_factor);
                rgb[1] = static_cast<u8>(rgb[1] * m_fade_factor);
                rgb[2] = static_cast<u8>(rgb[2] * m_fade_factor);
            }
        }

        fl_draw_image(offscreen_buffer.ptr(), 0, 0, width, height);

        const float factor = 1 / sqrtf(m_max_magnitude);
        const unsigned x_offset = width / 2;
        const unsigned y_offset = height / 2;
        const float scale = std::min(width, height) * .48f;

        fl_color(FL_GREEN);
        std::optional<Point> previous;
        for (auto& to_draw : m_sample_buffer) {
            to_draw *= factor;
            const Point next(to_draw.real() * scale + x_offset, to_draw.imag() * scale + y_offset);

            if (m_connect) {
                if (previous)
                    fl_line(previous->x(), previous->y(), next.x(), next.y());

                previous = next;
            } else {
                fl_point(next.x(), next.y());
            }
        }

        m_sample_count = 0;
        m_max_magnitude = 0;
        damage(FL_DAMAGE_ALL);
    }
}
