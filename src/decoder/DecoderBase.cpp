#include "Decoder.hpp"
#include <Drtd.hpp>
#include <ui/MainGui.hpp>
#include <ui/component/Waterfall.hpp>
#include <util/Config.hpp>

using namespace Dsp;

constexpr const char* conf_waterfall_settings { ".Base.WaterfallSettings" };
constexpr const char* conf_center_frequency { ".Base.CenterFrequency" };

void DecoderBase::save_ui_settings() {
    Util::Config::save(m_config_path + conf_waterfall_settings, Drtd::main_gui().waterfall().settings());
    Util::Config::save(m_config_path + conf_center_frequency, m_center_frequency);
}

void DecoderBase::load_ui_settings() {
    auto settings = Drtd::main_gui().waterfall().settings();
    Util::Config::load(m_config_path + conf_waterfall_settings, settings, settings);
    Drtd::main_gui().waterfall().update_settings_later(settings);
    Util::Config::load(m_config_path + conf_center_frequency, m_center_frequency, m_center_frequency);
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
