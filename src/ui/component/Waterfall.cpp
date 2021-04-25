#include "Waterfall.hpp"

#include <decoder/Decoder.hpp>
#include <ui/WaterfallDialog.hpp>
#include <util/Logger.hpp>
#include <util/Resampler.hpp>
#include <util/Util.hpp>

using namespace Ui;

static const Util::Logger s_log("Waterfall");
static constexpr u8 scale_area_height { 30 };
static constexpr u8 scale_marker_major_height { 10 };
static constexpr u8 scale_marker_minor_height { 5 };
static constexpr u16 scroll_frequency_increment { 500 };

static const Fl_Color s_colorful_colors[] = {
    FL_BLACK,
    fl_darker(FL_BLUE),
    fl_darker(FL_CYAN),
    FL_CYAN,
    FL_GRAY0,
    FL_GREEN,
    FL_YELLOW,
    Util::s_amber_color,
    FL_RED
};
static const Fl_Color s_amber_colors[] = { FL_BLACK, Util::s_amber_color };

static const Util::Buffer<Palette::WaterfallPalette> s_palettes {
    { "Colorful", s_colorful_colors, sizeof(s_colorful_colors) / sizeof(Fl_Color) },
    { "Amber", s_amber_colors, sizeof(s_amber_colors) / sizeof(Fl_Color) }
};

const Util::Buffer<Palette::WaterfallPalette>& Palette::palettes() {
    return s_palettes;
}

Waterfall::Waterfall(GlobalSettings settings, u32 x, u32 y, u32 w, u32 h)
    : Canvas(x, y, w, h, 2)
    , m_new_window_index(settings.window_index)
    , m_global_settings(settings)
    , m_samples(m_settings.bins)
    , m_window_coefficients(m_settings.bins) {
    m_waterfall_layer = make_layer(0, scale_area_height, Layer::parent_size, Layer::parent_size);
    m_scale_layer = make_layer(0, 0, Layer::parent_size, scale_area_height);
    update_settings(true);
    box(FL_DOWN_BOX);
    redraw_scale_later();
}

void Waterfall::show_marker(bool show) {
    m_show_marker = show;
    redraw_scale_and_marker();
    redraw();
}

void Waterfall::update_settings_later(const Settings& settings) {
    std::unique_lock<std::mutex> lock(m_settings_mutex);
    m_new_settings = settings;
    m_update_settings = true;
    force_redraw();
}

void Waterfall::force_redraw() {
    redraw_scale_and_marker();
    damage(FL_DAMAGE_ALL);
}

void Waterfall::set_decoder(std::shared_ptr<Dsp::DecoderBase>& decoder) {
    m_decoder = decoder;
    redraw_scale_later();
}

void Waterfall::redraw_scale_later() {
    m_redraw_scale = true;
}

void Waterfall::set_sample_rate(u16 sample_rate) {
    if (sample_rate == m_sample_rate)
        return;

    m_sample_rate = sample_rate;
    redraw_scale_later();
}

int Waterfall::handle(int event) {
    if (m_scale_layer->current_width() != m_old_size.w() || m_scale_layer->current_height() != m_old_size.h())
        force_redraw();

    if (!m_decoder)
        return Canvas::handle(event);

    if (Fl::event_button3() && event == FL_PUSH) {
        WaterfallDialog::show_dialog();
        return 1;
    } else if (Fl::event_button1() && (event == FL_PUSH || event == FL_DRAG)) {
        const auto freq = static_cast<Hertz>(Waterfall::translate_x_to_hz(m_settings, Fl::event_x() - x() - padding() + m_settings.bin_offset, m_sample_rate));
        if (event == FL_PUSH)
            s_log.info() << "Clicked at " << freq << " Hz";

        if (m_decoder->marker().moveable && m_input_limiter.limit())
            m_decoder->set_center_frequency(std::max(m_decoder->min_center_frequency(), std::min(static_cast<u16>(m_sample_rate / 2), freq)));
        return 1;
    } else if (event == FL_MOUSEWHEEL) {
        auto new_settings = settings();
        auto dy = Fl::event_dy();
        auto step = translate_hz_to_x(settings(), scroll_frequency_increment, m_sample_rate);

        if (Fl::event_shift()) {
            if (dy < 0) {
                /* Up */
                new_settings.zoom += .5f;
                new_settings.zoom = std::min(new_settings.zoom, WaterfallDialog::s_max_zoom);
            } else {
                /* Down */
                new_settings.zoom -= .5f;
                new_settings.zoom = std::max(new_settings.zoom, WaterfallDialog::s_min_zoom);
            }

            new_settings.bin_offset = static_cast<u32>(Waterfall::translate_hz_to_x(new_settings, static_cast<Hertz>(Waterfall::translate_x_to_hz(settings(), new_settings.bin_offset, m_sample_rate)), m_sample_rate));
        } else {
            if (dy < 0) {
                /* Up */
                new_settings.bin_offset = std::min(static_cast<u32>(translate_hz_to_x(settings(), m_sample_rate / 2, m_sample_rate)), new_settings.bin_offset + step);
            } else {
                /* Down */
                if (new_settings.bin_offset < static_cast<u32>(step))
                    new_settings.bin_offset = 0;
                else
                    new_settings.bin_offset -= step;
            }
        }

        update_settings_later(new_settings);
        WaterfallDialog::load_from_waterfall(new_settings);
        return 1;
    }

    return Canvas::handle(event);
}

void Waterfall::update_settings(bool force) {
    std::unique_lock<std::mutex> lock(m_settings_mutex);
    auto bins = m_new_settings.bins;
    bool bins_changed = bins != m_settings.bins || force;

    if (bins_changed) {
        m_samples.resize(bins);
        m_window_coefficients = Util::Buffer<float>(bins);
        m_fft = FFT(bins);
    }

    if (m_global_settings.window_index != m_new_window_index || bins_changed) {
        Dsp::Window::s_windows[m_new_window_index].calculate_coefficients(m_window_coefficients);
        m_global_settings.window_index = m_new_window_index;
    }

    m_settings = m_new_settings;
}

void Waterfall::process_sample(float sample) {
    m_samples.push(sample);

    if (m_sample_count < m_settings.bins / (m_settings.speed_multiplier + 1)) {
        ++m_sample_count;
        return;
    }

    m_sample_count = 0;
    if (!m_waterfall_layer->current_width() || !m_waterfall_layer->current_height())
        return;

    for (size_t i = 0; i < m_settings.bins; ++i)
        m_fft.input_buffer()[i] = m_samples.peek(i) * m_window_coefficients[i];

    m_fft.execute();

    float max = 0;
    u32 downsample_index = 0;
    auto pseudo_bins = Waterfall::pseudo_bins(m_settings);
    bool down_sampling = Waterfall::down_sampling(m_settings);
    Util::Resampler down_sampler(std::abs(m_settings.zoom) + 1, 1);
    Util::Buffer<float> bin_values(down_sampling ? pseudo_bins / 2 : m_settings.bins / 2);

    for (size_t i = 0; i < m_settings.bins / 2; ++i) {
        float real = static_cast<float>(m_fft.output_buffer()[i][0]);
        if (std::isinf(real))
            real = 0;

        float imag = static_cast<float>(m_fft.output_buffer()[i][1]);
        if (std::isinf(imag))
            imag = 0;

        float magnitude = real * real + imag * imag;
        if (!m_settings.power_spectrum)
            magnitude = sqrtf(magnitude);

        if (down_sampling) {
            down_sampler.process_input_sample(magnitude);
            if (down_sampler.read_output_sample(magnitude)) {
                max = std::max(max, magnitude);
                if (downsample_index >= bin_values.size())
                    break;

                bin_values[downsample_index++] = magnitude;
            }
        } else {
            bin_values[i] = magnitude;
            max = std::max(max, magnitude);
        }
    }

    Util::Buffer<u8> line_buffer(m_waterfall_layer->current_width() * 3);
    auto& palette = Ui::Palette::palettes()[m_global_settings.palette_index];

    unsigned x = 0;
    for (size_t i = m_settings.bin_offset; i < pseudo_bins / 2 && x < m_waterfall_layer->current_width(); ++i, ++x) {
        float value;
        if (down_sampling || m_settings.zoom == 0)
            value = bin_values[i];
        else
            value = Util::linear_interpolate(i / static_cast<float>(pseudo_bins) * m_settings.bins, bin_values);

        if (std::isnan(value))
            value = 0;

        const float float_index = Util::scale_log(max == 0 ? 0 : value / max, 0, 1, true) * static_cast<float>(palette.color_count - 1);
        const size_t base_index = static_cast<size_t>(float_index);
        const auto base_color = palette.colors[base_index];
        const auto mix_color = palette.colors[std::min(palette.color_count - 1, base_index + 1)];
        const float mix_amount = float_index - static_cast<float>(base_index);
        const float base_amount = 1 - mix_amount;

        uchar base_red = 0;
        uchar base_green = 0;
        uchar base_blue = 0;

        uchar mix_red = 0;
        uchar mix_green = 0;
        uchar mix_blue = 0;

        Fl::get_color(base_color, base_red, base_green, base_blue);
        Fl::get_color(mix_color, mix_red, mix_green, mix_blue);

        const size_t li = x * 3;
        line_buffer[li] = static_cast<u8>((mix_amount * mix_red) + (base_amount * base_red));
        line_buffer[li + 1] = static_cast<u8>((mix_amount * mix_green) + (base_amount * base_green));
        line_buffer[li + 2] = static_cast<u8>((mix_amount * mix_blue) + (base_amount * base_blue));
    }

    {
        Ui::LayerDraw draw(m_waterfall_layer);

        fl_copy_offscreen(0, 1, m_waterfall_layer->current_width(), m_waterfall_layer->current_height() - 1, m_waterfall_layer->offscreen_buffer(), 0, 0);
        fl_draw_image(line_buffer.ptr(), 0, 0, x, 1);
        fl_color(FL_GRAY);
        fl_line(x, 0, m_waterfall_layer->current_width(), 0);
    }

    if (m_update_settings) {
        update_settings(false);
        m_update_settings = false;
    }

    if (m_redraw_scale || m_old_width != w()) {
        redraw_scale_and_marker();
        m_old_width = w();
        m_redraw_scale = false;
    }

    if (m_redraw_limiter.limit())
        damage(FL_DAMAGE_ALL);
}

void Waterfall::redraw_scale_and_marker() {
    if (!m_scale_layer->current_width())
        return;

    m_old_size = { m_scale_layer->current_width(), m_scale_layer->current_height() };
    const auto bin_offset = m_new_settings.bin_offset;
    u32 last_frequency = static_cast<u32>(Waterfall::translate_x_to_hz(m_new_settings, bin_offset, m_sample_rate) / 500);
    Ui::LayerDraw draw(m_scale_layer);

    fl_rectf(0, 0, m_scale_layer->current_width(), scale_area_height, FL_BLACK);
    fl_color(FL_WHITE);
    fl_font(FL_COURIER | FL_BOLD, 16);

    u32 x = 0;
    for (; x < Waterfall::pseudo_bins(m_new_settings) / 2 && x < m_scale_layer->current_width(); ++x) {
        u32 frequency_x = static_cast<u32>(Waterfall::translate_x_to_hz(m_new_settings, x + bin_offset, m_sample_rate));
        auto frequency_stepped = frequency_x / 500;

        if (frequency_stepped != last_frequency) {
            u16 marker_top = scale_area_height - ((frequency_stepped % 2 == 0) ? scale_marker_major_height : scale_marker_minor_height);
            fl_line(x, marker_top, x, scale_area_height);

            if (frequency_stepped % 2 == 0) {
                auto text = std::to_string(frequency_stepped / 2);
                u16 center_x = static_cast<u16>(std::round(fl_width(text.c_str()) / 2));
                fl_draw(text.c_str(), x - center_x, marker_top - 2);
            }
            last_frequency = frequency_stepped;
        }

        if (frequency_x > m_sample_rate / 2)
            break;
    }

    fl_color(FL_RED);
    fl_line(x, 0, x, scale_area_height);

    if (!m_decoder || !m_decoder->marker().markers.size() || !m_show_marker)
        return;

    const int marker_center_x = Waterfall::translate_hz_to_x(m_new_settings, m_decoder->center_frequency(), m_sample_rate) - bin_offset;
    if (marker_center_x >= 0) {
        fl_color(FL_YELLOW);
        fl_line(marker_center_x, 0, marker_center_x, scale_area_height);
    }

    unsigned mark_height = static_cast<unsigned>(scale_area_height * .2);
    fl_color(FL_RED);

    for (const auto& mark : m_decoder->marker().markers) {
        i32 marker_bandwidth_x = Waterfall::translate_hz_to_x(m_new_settings, mark.bandwidth, m_sample_rate);
        int marker_offset_x = Waterfall::translate_hz_to_x(m_new_settings, mark.offset, m_sample_rate);
        int marker_center_x_abs = marker_center_x + marker_offset_x;

        if (marker_center_x_abs >= 0) {
            fl_rectf(marker_center_x_abs - marker_bandwidth_x / 2, scale_area_height - mark_height, marker_bandwidth_x, mark_height);
            fl_line(marker_center_x_abs, static_cast<unsigned>(scale_area_height * .4f), marker_center_x_abs, scale_area_height - 1);
        }
    }
}
