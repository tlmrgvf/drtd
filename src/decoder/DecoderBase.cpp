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
#include "Decoder.hpp"
#include <Drtd.hpp>
#include <ui/MainGui.hpp>
#include <ui/component/Waterfall.hpp>
#include <util/Config.hpp>

using namespace Dsp;

constexpr const char* conf_waterfall_settings { "Base.WaterfallSettings" };
constexpr const char* conf_center_frequency { "Base.CenterFrequency" };

void DecoderBase::save_ui_settings() {
    Util::Config::save(config_path(conf_waterfall_settings), Drtd::main_gui().waterfall().settings());
    Util::Config::save(config_path(conf_center_frequency), m_center_frequency);
}

void DecoderBase::load_ui_settings() {
    auto settings = Drtd::main_gui().waterfall().settings();
    Util::Config::load(config_path(conf_waterfall_settings), settings, settings);
    Drtd::main_gui().waterfall().update_settings_later(settings);
    Util::Config::load(config_path(conf_center_frequency), m_center_frequency, m_center_frequency);
    set_center_frequency(m_center_frequency);
}

void DecoderBase::set_marker(Util::MarkerGroup marker) {
    m_marker = std::move(marker);
    m_min_center_frequency = 0;
    for (auto& mark : m_marker.markers) {
        if (mark.offset >= 0)
            continue;

        m_min_center_frequency = std::max(m_min_center_frequency, static_cast<Hertz>(-mark.offset + mark.bandwidth / 2));
    }

    m_center_frequency = std::max(m_center_frequency, m_min_center_frequency);
    if (Drtd::using_ui())
        Drtd::main_gui().waterfall().force_redraw();
}

void DecoderBase::set_center_frequency(Hertz center_frequency) {
    m_center_frequency = std::clamp(center_frequency, m_min_center_frequency, static_cast<Hertz>(m_input_sample_rate / 2));
    if (Drtd::using_ui()) {
        Drtd::main_gui().update_center_frequency();
        on_marker_move(center_frequency);
    }
}

void DecoderBase::set_status(const std::string& status) {
    if (Drtd::using_ui())
        Drtd::main_gui().set_status(status);
}
