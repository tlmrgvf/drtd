#include "MovingAverageDialog.hpp"
#include <Drtd.hpp>
#include <FL/Fl_Window.H>
#include <dsp/MovingAverage.hpp>
#include <thread/ProcessingThread.hpp>
#include <ui/MainGui.hpp>
#include <util/FFT.hpp>
#include <util/Util.hpp>

using namespace Ui;

static ConfigRef<Dsp::MovingAverageBase> s_current_filter;

MovingAverageDialog::MovingAverageDialog()
    : Fl_Window(0, 0, 420, 268, "Moving average settings") {
    icon(Drtd::drtd_icon());
    size_range(400, 200);
    m_plot_container = new Fl_Group(2, 16, w() - 4, 200, "Frequency response");
    m_plot = new FrequencyPlot(m_plot_container->x() + 4, m_plot_container->y() + 4, m_plot_container->w() - 8, m_plot_container->h() - 8);
    m_plot_container->resizable(m_plot);
    m_plot_container->box(FL_ENGRAVED_BOX);
    m_plot_container->align(FL_ALIGN_TOP_LEFT);
    m_plot_container->labelfont(FL_BOLD);
    m_plot_container->labelsize(12);
    m_plot_container->end();

    auto settings_y = m_plot_container->y() + m_plot_container->h() + 20;
    m_settings_container = new Fl_Group(m_plot_container->x(), settings_y, w() - 4, 30, "Moving average settings");
    m_taps = new Fl_Spinner(m_settings_container->x() + 52, settings_y + 4, m_settings_container->w() - 56, 22, "Taps:");
    m_taps->maximum(8001);
    m_taps->minimum(1);

    m_settings_container->box(FL_ENGRAVED_BOX);
    m_settings_container->align(FL_ALIGN_TOP_LEFT);
    m_settings_container->labelfont(FL_BOLD);
    m_settings_container->labelsize(12);
    m_settings_container->end();

    resizable(m_plot_container);
    end();

    m_taps->callback(MovingAverageDialog::update_filter);
}

void MovingAverageDialog::close_dialog() {
    if (s_moving_average_dialog.has_instance()) {
        s_current_filter = {};
        s_moving_average_dialog->hide();
    }
}

void MovingAverageDialog::show_dialog(ConfigRef<Dsp::MovingAverageBase> filter) {
    auto& main_gui = Drtd::main_gui();
    s_moving_average_dialog->position(
        main_gui.x() + Util::center(main_gui.w(), s_moving_average_dialog->w()),
        main_gui.y() + Util::center(main_gui.h(), s_moving_average_dialog->h()));
    s_moving_average_dialog->show();
    s_current_filter = filter;
    MovingAverageDialog::update_dialog();
}

void MovingAverageDialog::update_filter(Fl_Widget*) {
    Dsp::ProcessingLock lock;
    s_current_filter->set_taps(static_cast<Taps>(s_moving_average_dialog->m_taps->value()));
    update_dialog();
}

void MovingAverageDialog::update_dialog() {
    if (s_moving_average_dialog.has_instance() && s_moving_average_dialog->visible() && s_current_filter.valid()) {
        const Taps taps = s_current_filter->taps();
        s_moving_average_dialog->m_taps->value(taps);
        constexpr Hertz min_fft_hz_per_bin = 1;
        auto bins = std::max(taps * 2, s_current_filter->sample_rate() / min_fft_hz_per_bin);
        FFT fft(bins);
        std::fill(fft.input_buffer().begin(), fft.input_buffer().end(), 0);
        std::fill(fft.input_buffer().begin(), fft.input_buffer().begin() + taps, 1 / static_cast<float>(taps));
        fft.execute();

        bins &= ~static_cast<Taps>(1);
        Buffer<float> plot_values(bins / 2);
        float min_value = std::numeric_limits<float>::max();
        float max_value = std::numeric_limits<float>::min();
        for (size_t i = 0; i < plot_values.size(); ++i) {
            float real = static_cast<float>(fft.output_buffer()[i][0]);
            if (std::isinf(real))
                real = 0;

            float imag = static_cast<float>(fft.output_buffer()[i][1]);
            if (std::isinf(imag))
                imag = 0;

            const float magnitude = sqrtf(real * real + imag * imag);
            min_value = std::min(min_value, magnitude);
            max_value = std::max(max_value, magnitude);
            plot_values[i] = magnitude;
        }

        s_moving_average_dialog->m_plot->plot(std::move(plot_values), s_current_filter->sample_rate() / 2, min_value, max_value);
    }
}
