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
#include "BiquadFilterDialog.hpp"
#include <Drtd.hpp>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Window.H>
#include <dsp/BiquadFilterComponent.hpp>
#include <thread/ProcessingThread.hpp>
#include <ui/ConfigDialog.hpp>
#include <ui/MainGui.hpp>
#include <util/FFT.hpp>
#include <util/Util.hpp>

using namespace Ui;

static ConfigRef<Dsp::Biquad::FilterBase> s_current_filter;

BiquadFilterDialog::BiquadFilterDialog()
    : Fl_Window(0, 0, 420, 316, "Biquad filter settings") {
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
    m_settings_container = new Fl_Group(m_plot_container->x(), settings_y, w() - 4, 78, "Filter settings");
    m_center = new Fl_Spinner(m_settings_container->x() + 140, settings_y + 4, m_settings_container->w() - 144, 22, "Center:");

    m_parameter = new Fl_Spinner(m_center->x(), m_center->y() + m_center->h() + 2, m_center->w(), m_center->h(), "Q:");
    m_parameter->range(-100, 100);
    m_parameter->step(.001);

    m_type = new Fl_Choice(m_parameter->x(), m_parameter->y() + m_parameter->h() + 2, m_parameter->w(), m_parameter->h(), "Type:");
    for (const auto& name : Dsp::Biquad::Coefficients::names)
        m_type->add(name);

    m_settings_container->box(FL_ENGRAVED_BOX);
    m_settings_container->align(FL_ALIGN_TOP_LEFT);
    m_settings_container->labelfont(FL_BOLD);
    m_settings_container->labelsize(12);
    m_settings_container->end();

    resizable(m_plot_container);
    end();

    m_center->callback(BiquadFilterDialog::update_filter);
    m_parameter->callback(BiquadFilterDialog::update_filter);
    m_type->callback(BiquadFilterDialog::update_filter);
}

void BiquadFilterDialog::close_dialog() {
    if (s_biquad_filter_dialog.has_instance()) {
        s_current_filter = {};
        s_biquad_filter_dialog->hide();
    }
}

void BiquadFilterDialog::show_dialog(ConfigRef<Dsp::Biquad::FilterBase> filter) {
    auto& main_gui = Drtd::main_gui();
    s_biquad_filter_dialog->position(
        main_gui.x() + Util::center(main_gui.w(), s_biquad_filter_dialog->w()),
        main_gui.y() + Util::center(main_gui.h(), s_biquad_filter_dialog->h()));
    s_biquad_filter_dialog->show();
    s_current_filter = filter;
    BiquadFilterDialog::update_dialog();
}

void BiquadFilterDialog::update_filter(Fl_Widget*) {
    Dsp::ProcessingLock lock;
    auto& diag = *s_biquad_filter_dialog;
    auto& filter = *s_current_filter;

    filter.set_center(static_cast<float>(diag.m_center->value()));
    filter.set_parameter(static_cast<float>(diag.m_parameter->value()));
    filter.set_type(static_cast<Dsp::Biquad::Type>(diag.m_type->value()));
    filter.recalculate();

    update_dialog();
}

void BiquadFilterDialog::update_dialog() {
    if (s_biquad_filter_dialog.has_instance() && s_biquad_filter_dialog->visible() && s_current_filter.valid()) {
        auto& diag = *s_biquad_filter_dialog;
        auto& filter = *s_current_filter;

        diag.m_center->range(0, filter.sample_rate() / 2);
        diag.m_center->value(filter.center());
        diag.m_parameter->value(filter.parameter());
        diag.m_type->value(static_cast<int>(filter.type()));

        // FIXME: Label is not properly cleared, this is a hack
        diag.m_parameter->label("                                                            ");
        diag.m_parameter->copy_label((Dsp::Biquad::Coefficients::parameter_description_for(filter.type()) + ":").c_str());

        constexpr u16 plot_hz_step = 10;
        Buffer<float> plot_values(filter.sample_rate() / plot_hz_step / 2);
        Dsp::Biquad::Coefficients coefficients(filter.type(), filter.sample_rate(), filter.center(), filter.parameter());

        float min_value = std::numeric_limits<float>::max();
        float max_value = std::numeric_limits<float>::min();
        for (size_t i = 0; i < plot_values.size(); ++i) {
            const float att = coefficients.response_at(static_cast<Hertz>(i * plot_hz_step));
            if (att > 0)
                min_value = std::min(min_value, att);

            max_value = std::max(max_value, att);
            plot_values[i] = att;
        }

        diag.m_plot->plot(std::move(plot_values), filter.sample_rate() / 2, min_value, max_value);
    }

    Ui::ConfigDialog::refresh();
}
