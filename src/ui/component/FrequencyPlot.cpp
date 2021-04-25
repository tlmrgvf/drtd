#include "FrequencyPlot.hpp"
#include <Fl/fl_draw.H>
#include <cmath>
#include <iomanip>
#include <ios>
#include <optional>
#include <sstream>
#include <util/Point.hpp>
#include <util/Util.hpp>

using namespace Ui;

FrequencyPlot::FrequencyPlot(u32 x, u32 y, u32 w, u32 h)
    : Canvas(x, y, w, h, 0) {
    m_plot_layer = make_layer(0, 0, Layer::parent_size, Layer::parent_size, fl_rgb_color(25, 0, 45));
    tooltip("Middle mouse click to switch between log/linear scale\nScroll to adjust dB/div");
}

int FrequencyPlot::handle(int event) {
    bool redraw = false;

    if (Fl::event_button2() && event == FL_PUSH) {
        m_plot_linear = !m_plot_linear;
        redraw = true;
    } else if (Fl::event_button1() && (event == FL_PUSH || event == FL_DRAG)) {
        m_marker_x = Fl::event_x() - x() - padding();
        m_show_marker = Fl::event_clicks() == 0;
        redraw = true;
    } else if (event == FL_MOUSEWHEEL) {
        auto dy = Fl::event_dy();

        if (dy < 0) {
            /* Up */
            m_vertical_scale = std::min(db_scale_max, m_vertical_scale + db_scale_step);
        } else {
            /* Down */
            m_vertical_scale = std::max(db_scale_min, m_vertical_scale - db_scale_step);
        }

        redraw = true;
    }

    if (m_plot_layer->current_width() != m_old_size.w() || m_plot_layer->current_height() != m_old_size.h() || redraw) {
        plot();
        return 1;
    }

    return Canvas::handle(event);
}

void FrequencyPlot::plot(Buffer<float> values, float max_frequency, float min_value, float max_value, float reference) {
    m_values = std::move(values);
    m_min_value = min_value;
    m_max_value = max_value;
    m_reference = reference;
    m_max_frequency = max_frequency;
    plot();
}

void FrequencyPlot::plot() {
    fl_font(FL_COURIER | FL_BOLD, 14);

    constexpr unsigned info_text_offset = 6;
    const auto grid_color = fl_rgb_color(100, 100, 100);
    const auto plot_color = Util::s_amber_color;
    const unsigned text_height = fl_height() - fl_descent();
    const unsigned height = m_plot_layer->current_height();
    const unsigned width = m_plot_layer->current_width();
    const float db_max = -static_cast<float>(m_vertical_scale);
    const unsigned plot_offset_y = text_height + 2 * info_text_offset;
    const float min_att = Util::db_voltage(m_max_value, m_reference);
    const float max_att = Util::db_voltage(m_min_value, m_reference);
    unsigned plot_height = height - plot_offset_y - 2 * info_text_offset - 4;

    if (height < 20 || width < 20)
        return;

    Ui::LayerDraw drawer(m_plot_layer);

    fl_rectf(m_plot_layer->x(), m_plot_layer->y(), m_plot_layer->current_width(), m_plot_layer->current_height(), m_plot_layer->clear_color());
    fl_color(FL_WHITE);

    std::ostringstream string_stream;
    float min_att_rounded = std::round(min_att * 100) / 100.f;
    string_stream << std::setw(5) << std::setfill(' ') << std::right << min_att_rounded << "dB";
    const auto zero_marker = string_stream.str();

    const unsigned plot_offset_x = static_cast<unsigned>(fl_width(zero_marker.c_str()) + 4);
    const unsigned plot_width = width - plot_offset_x - 15;
    const float plot_width_f = static_cast<float>(width - plot_offset_x - 15);

    fl_draw(zero_marker.c_str(), 2, static_cast<int>(text_height / 2 + plot_offset_y));
    fl_color(grid_color);
    fl_line(plot_offset_x, plot_offset_y, plot_offset_x + plot_width, plot_offset_y);

    const float marker_step = static_cast<float>(plot_width) / frequency_divisions;
    unsigned last_x = 0;
    for (float x = 0; x <= frequency_divisions; ++x) {
        const float frequency = m_plot_linear ? x * marker_step * m_max_frequency / plot_width_f : Util::scale_log(x, 0, frequency_divisions) * m_max_frequency;

        string_stream = std::ostringstream();
        string_stream.precision(3);
        string_stream.fill(' ');
        string_stream.width(5);
        if (frequency >= 1000)
            string_stream << frequency / 1000. << " kHz";
        else
            string_stream << frequency << " Hz";

        const auto frequency_string = string_stream.str();
        const float string_width = static_cast<float>(fl_width(frequency_string.c_str()));

        /* Check if we can draw the label without interfering with other labels */
        const unsigned xt = static_cast<unsigned>(x * marker_step + static_cast<float>(plot_offset_x) + (static_cast<float>(text_height) / 2.f));
        if (xt - last_x < text_height + 2 && x != 0)
            continue;
        plot_height = std::min(plot_height, static_cast<unsigned>(static_cast<float>(height - plot_offset_y) - string_width));

        /* Draw label */
        fl_draw(90, frequency_string.c_str(), xt, height + info_text_offset - 8);
        last_x = xt;
    }

    plot_height -= 5;

    for (unsigned x = 0; x <= frequency_divisions; ++x) {
        const unsigned xl = static_cast<unsigned>(static_cast<float>(x) * marker_step + static_cast<float>(plot_offset_x));
        fl_line(xl, plot_offset_y, xl, plot_offset_y + plot_height);
    }

    const float att_step = static_cast<float>(plot_height / amplitude_divisions);
    for (unsigned y = 0; y <= amplitude_divisions; ++y) {
        const unsigned yl = static_cast<unsigned>(static_cast<float>(y) * att_step + static_cast<float>(plot_offset_y));
        fl_line(plot_offset_x, yl, plot_offset_x + plot_width, yl);
    }

    float att_at_marker = 0;
    float att_rel_at_marker = 0;

    if (!m_values.size())
        return;

    m_marker_x = std::clamp(m_marker_x, plot_offset_x, plot_offset_x + plot_width - 1);
    const unsigned marker_x_offset = m_marker_x - plot_offset_x;

    fl_color(plot_color);
    std::optional<Point> last;
    const int lower_line = plot_height + plot_offset_y;
    for (unsigned x = 0; x < plot_width; ++x) {
        float log_index;
        if (m_plot_linear)
            log_index = x / static_cast<float>(plot_width - 1);
        else
            log_index = Util::scale_log(x, 0, plot_width - 1);

        const float value = Util::linear_interpolate(log_index * (m_values.size() - 1), m_values);
        const float attenuation = Util::db_voltage(value, m_reference) - min_att;

        if (x == marker_x_offset) {
            att_at_marker = attenuation + min_att;
            att_rel_at_marker = Util::db_voltage(value, m_max_value);
        }

        const int y = (std::isinf(attenuation) ? plot_height : roundf(attenuation / db_max * plot_height)) + plot_offset_y;
        if (!last.has_value())
            last = Point(x, y);

        if (y >= lower_line && last->y() > lower_line) {
            last->set_x(x);
            continue;
        }

        fl_line(last->x() + plot_offset_x, std::min(last->y(), lower_line), x + plot_offset_x, std::min(y, lower_line));
        last->set_x(x);
        last->set_y(y);
    }

    const float marker_frequency = (m_plot_linear ? static_cast<float>(marker_x_offset) * m_max_frequency / static_cast<float>(plot_width)
                                                  : Util::scale_log(static_cast<float>(marker_x_offset), 0, static_cast<float>(plot_width)) * m_max_frequency);
    const bool shorten_marker_frequency = marker_frequency >= 1000;

    string_stream = std::ostringstream();
    string_stream.precision(3);
    string_stream << "Marker: " << att_at_marker << " dB (" << att_rel_at_marker << " dBc) @ "
                  << (shorten_marker_frequency ? marker_frequency / 1000. : marker_frequency) << " " << (shorten_marker_frequency ? "KHz" : "Hz");

    const auto marker_string = string_stream.str();
    const float db_per_div = -db_max / amplitude_divisions;

    string_stream = std::ostringstream();
    string_stream.precision(3);
    string_stream << "| Max. Att.: " << max_att << " dB | dB/div: " << std::setw(4) << std::left << db_per_div << " | ";
    if (m_show_marker)
        string_stream << marker_string;

    fl_color(FL_WHITE);
    fl_draw(string_stream.str().c_str(), plot_offset_x, text_height + info_text_offset);

    if (m_show_marker) {
        fl_line_style(FL_DASH);
        fl_color(FL_YELLOW);
        fl_line(m_marker_x, plot_offset_y, m_marker_x, plot_offset_y + plot_height);
        fl_line_style(0);
    }

    m_old_size = { width, height };
    damage(FL_DAMAGE_ALL);
}
