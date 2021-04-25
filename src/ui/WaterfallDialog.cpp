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
#include "WaterfallDialog.hpp"
#include <Drtd.hpp>
#include <sstream>
#include <ui/MainGui.hpp>
#include <util/Singleton.hpp>
#include <util/Util.hpp>

using namespace Ui;

const std::array s_bins { "1024", "2048", "4096", "8192", "16384", "32768", "65536" };

WaterfallDialog::WaterfallDialog()
    : Fl_Window(100, 100, 360, 204, "Waterfall settings")
    , m_window(85, 4, 0, 25, "Window:")
    , m_zoom(m_window.x(), m_window.y() + m_window.h() + 4, w() - m_window.x() - 24, 25, "Zoom:")
    , m_reset_zoom(m_zoom.x() + m_zoom.w() + 4, m_zoom.y(), 15, m_zoom.h(), "R")
    , m_bin_offset(m_window.x(), m_zoom.y() + m_zoom.h() + 4, 150, 25, "Offset (Hz):")
    , m_speed_multiplier(m_window.x(), m_bin_offset.y() + m_bin_offset.h() + 4, w() - m_window.x() - 4, 25, "Speed:")
    , m_bins(m_window.x(), m_speed_multiplier.y() + m_speed_multiplier.h() + 4, 150, 25, "Bins:")
    , m_palette(m_window.x(), m_bins.y() + m_bins.h() + 4, 0, 25, "Palette:")
    , m_power_spectrum(m_window.x(), m_palette.y() + m_palette.h() + 4, 180, 25, "Show power spectrum") {
    double max_width = 0;

    for (auto& window : Dsp::Window::s_windows) {
        m_window.add(window.name().c_str());
        max_width = std::max(fl_width(window.name().c_str()), max_width);
    }

    m_window.size(static_cast<int>(max_width + 40), m_window.h());
    m_window.align(FL_ALIGN_LEFT);
    m_window.callback(save_to_waterfall);

    m_zoom.type(FL_HOR_NICE_SLIDER);
    m_zoom.selection_color(Util::s_amber_color);
    m_zoom.align(FL_ALIGN_LEFT);
    m_zoom.bounds(s_min_zoom, s_max_zoom);
    m_zoom.callback(save_to_waterfall);

    m_reset_zoom.callback([](Fl_Widget*, void*) {
        s_waterfall_dialog->m_zoom.value(0);
        save_to_waterfall(nullptr, nullptr);
    });

    m_bin_offset.align(FL_ALIGN_LEFT);
    m_bin_offset.callback(save_to_waterfall);

    m_speed_multiplier.type(FL_HOR_NICE_SLIDER);
    m_speed_multiplier.selection_color(Util::s_amber_color);
    m_speed_multiplier.align(FL_ALIGN_LEFT);
    m_speed_multiplier.callback(save_to_waterfall);
    m_speed_multiplier.bounds(0, 10);

    m_bins.align(FL_ALIGN_LEFT);
    for (auto bins : s_bins)
        m_bins.add(bins);
    m_bins.callback(save_to_waterfall);

    max_width = 0;
    for (auto& palette : Ui::Palette::palettes()) {
        m_palette.add(palette.name);
        max_width = std::max(fl_width(palette.name), max_width);
    }

    m_palette.size(static_cast<int>(max_width + 40), m_palette.h());
    m_palette.callback(save_to_waterfall);

    m_power_spectrum.callback(save_to_waterfall);
}

void WaterfallDialog::show_dialog() {
    auto& main_gui = Drtd::main_gui();
    s_waterfall_dialog->position(
        main_gui.x() + Util::center(main_gui.w(), s_waterfall_dialog->w()),
        main_gui.y() + Util::center(main_gui.h(), s_waterfall_dialog->h()));

    s_waterfall_dialog->load_from_waterfall(Drtd::main_gui().waterfall().settings());
    s_waterfall_dialog->set_non_modal();
    s_waterfall_dialog->show();
}

void WaterfallDialog::save_to_waterfall(Fl_Widget*, void*) {
    Waterfall::Settings new_settings;
    auto& waterfall = Drtd::main_gui().waterfall();

    new_settings.zoom = static_cast<float>(s_waterfall_dialog->m_zoom.value());
    new_settings.speed_multiplier = static_cast<u8>(s_waterfall_dialog->m_speed_multiplier.value());
    new_settings.bins = std::atoi(s_bins[static_cast<u32>(s_waterfall_dialog->m_bins.value())]);
    new_settings.power_spectrum = static_cast<bool>(s_waterfall_dialog->m_power_spectrum.value());
    s_waterfall_dialog->update_offset_spinner_limits(new_settings);
    new_settings.bin_offset = static_cast<u32>(Waterfall::translate_hz_to_x(new_settings, static_cast<Hertz>(s_waterfall_dialog->m_bin_offset.value()), waterfall.sample_rate()));

    waterfall.set_window_index(static_cast<u8>(s_waterfall_dialog->m_window.value()));
    waterfall.set_palette_index(static_cast<u8>(s_waterfall_dialog->m_palette.value()));
    waterfall.update_settings_later(new_settings);
}

void WaterfallDialog::update_offset_spinner_limits(const Waterfall::Settings& settings) {
    auto& waterfall = Drtd::main_gui().waterfall();
    m_bin_offset.step(Waterfall::hz_per_bin(settings, waterfall.sample_rate()));
    m_bin_offset.range(0, waterfall.sample_rate() / 2);
}

void WaterfallDialog::load_from_waterfall(const Waterfall::Settings& settings) {
    if (!s_waterfall_dialog.has_instance())
        return;

    auto& waterfall = Drtd::main_gui().waterfall();

    s_waterfall_dialog->m_window.value(waterfall.window_index());
    s_waterfall_dialog->m_zoom.value(settings.zoom);
    s_waterfall_dialog->m_speed_multiplier.value(settings.speed_multiplier);
    s_waterfall_dialog->update_offset_spinner_limits(settings);
    s_waterfall_dialog->m_bin_offset.value(Waterfall::translate_x_to_hz(settings, settings.bin_offset, waterfall.sample_rate()));
    s_waterfall_dialog->m_bins.value(s_waterfall_dialog->m_bins.find_item(std::to_string(settings.bins).c_str()));
    s_waterfall_dialog->m_palette.value(waterfall.palette_index());
    s_waterfall_dialog->m_power_spectrum.value(settings.power_spectrum);
}
