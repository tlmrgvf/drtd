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
#include "FirFilterDialog.hpp"
#include <Drtd.hpp>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Window.H>
#include <dsp/FirFilter.hpp>
#include <thread/ProcessingThread.hpp>
#include <ui/ConfigDialog.hpp>
#include <ui/MainGui.hpp>
#include <util/FFT.hpp>
#include <util/Util.hpp>

using namespace Ui;

static ConfigRef<Dsp::FirFilterBase> s_current_filter;

FirFilterDialog::FirFilterDialog()
    : Fl_Window(0, 0, 420, 362, "FIR filter settings") {
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
    m_settings_container = new Fl_Group(m_plot_container->x(), settings_y, w() - 4, 124, "Filter settings");
    m_taps = new Fl_Spinner(m_settings_container->x() + 130, settings_y + 4, m_settings_container->w() - 134, 22, "Filter taps:");
    m_taps->maximum(4001);
    m_taps->minimum(1);
    m_taps->step(2);

    m_start_frequency = new Fl_Spinner(m_taps->x(), m_taps->y() + m_taps->h() + 2, m_taps->w(), m_taps->h(), "Frequency start:");
    m_start_frequency->minimum(0);
    m_start_frequency->maximum(std::numeric_limits<u16>::max());

    m_stop_frequency = new Fl_Spinner(m_taps->x(), m_start_frequency->y() + m_start_frequency->h() + 2, m_taps->w(), m_taps->h(), "Frequency stop:");
    m_stop_frequency->minimum(0);
    m_stop_frequency->maximum(std::numeric_limits<u16>::max());

    m_window = new Fl_Choice(m_taps->x(), m_stop_frequency->y() + m_stop_frequency->h() + 2, m_taps->w(), m_taps->h(), "Window:");
    for (auto& window : Dsp::Window::s_windows)
        m_window->add(window.name().c_str());

    m_invert = new Fl_Check_Button(m_window->x(), m_window->y() + m_window->h() + 2, m_taps->w(), m_taps->h(), "Invert");

    m_settings_container->box(FL_ENGRAVED_BOX);
    m_settings_container->align(FL_ALIGN_TOP_LEFT);
    m_settings_container->labelfont(FL_BOLD);
    m_settings_container->labelsize(12);
    m_settings_container->end();

    resizable(m_plot_container);
    end();

    m_taps->callback([](auto*) {FirFilterDialog::update_filter(nullptr); FirFilterDialog::update_dialog(); });
    m_start_frequency->callback(FirFilterDialog::update_filter);
    m_stop_frequency->callback(FirFilterDialog::update_filter);
    m_invert->callback(FirFilterDialog::update_filter);
    m_window->callback(FirFilterDialog::update_filter);
}

void FirFilterDialog::update_dialog() {
    if (s_fir_filter_dialog.has_instance() && s_fir_filter_dialog->visible() && s_current_filter.valid()) {
        auto& diag = *s_fir_filter_dialog;
        auto& filter = *s_current_filter;

        diag.m_taps->value(filter.taps());
        diag.m_start_frequency->value(filter.start_frequency());
        diag.m_stop_frequency->value(filter.stop_frequency());
        diag.m_invert->value(filter.is_band_stop());
        diag.m_window->value(static_cast<int>(filter.window_type()));

        constexpr u16 min_fft_hz_per_bin = 10;
        auto sinc_coeffs = filter.coefficients().resized(std::max(filter.taps(), static_cast<u16>(filter.sample_rate() / min_fft_hz_per_bin)));
        Buffer<float> plot_values((sinc_coeffs.size() - 1) / 2);
        FFT fft(sinc_coeffs.size());
        for (size_t i = 0; i < sinc_coeffs.size(); ++i)
            fft.input_buffer()[i] = sinc_coeffs[i];

        fft.execute();
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

        diag.m_plot->plot(std::move(plot_values), filter.sample_rate() / 2, min_value, max_value);
    }

    ConfigDialog::refresh();
}

void FirFilterDialog::close_dialog() {
    if (s_fir_filter_dialog.has_instance()) {
        s_current_filter = {};
        s_fir_filter_dialog->hide();
    }
}

void FirFilterDialog::show_dialog(ConfigRef<Dsp::FirFilterBase> filter) {
    auto& main_gui = Drtd::main_gui();
    s_fir_filter_dialog->position(
        main_gui.x() + Util::center(main_gui.w(), s_fir_filter_dialog->w()),
        main_gui.y() + Util::center(main_gui.h(), s_fir_filter_dialog->h()));
    s_fir_filter_dialog->show();
    s_current_filter = filter;
    update_dialog();
}

void FirFilterDialog::update_filter(Fl_Widget*) {
    Dsp::ProcessingLock lock;
    auto& diag = *s_fir_filter_dialog;
    auto& filter = *s_current_filter;

    auto new_start_freq = diag.m_start_frequency->value();
    auto new_stop_freq = diag.m_stop_frequency->value();

    if (static_cast<Taps>(diag.m_taps->value()) % 2 == 0)
        diag.m_taps->value(diag.m_taps->value() + 1);

    diag.m_stop_frequency->range(new_start_freq + 1, static_cast<Hertz>(filter.sample_rate() / 2));
    diag.m_start_frequency->range(0, new_stop_freq - 1);

    auto properties = filter.properties();
    properties.start_frequency = static_cast<Hertz>(new_start_freq);
    properties.stop_frequency = static_cast<Hertz>(new_stop_freq);
    properties.taps = static_cast<Taps>(diag.m_taps->value());
    properties.band_stop = diag.m_invert->value();
    properties.window_type = static_cast<Dsp::WindowType>(diag.m_window->value());
    filter.set_properties(properties);
    update_dialog();
}
