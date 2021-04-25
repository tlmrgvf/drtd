#pragma once

#include <pipe/Component.hpp>
#include <string>
#include <util/Buffer.hpp>
#include <util/Marker.hpp>

namespace Dsp {

class DecoderBase {
public:
    enum class Headless : bool {
        Yes,
        No
    };

    DecoderBase(std::string name, SampleRate input_sample_rate, Headless headless, u16 min_ui_height)
        : m_name(name)
        , m_config_path("Decoder." + name)
        , m_input_sample_rate(input_sample_rate)
        , m_headless(headless)
        , m_log("Decoder <" + name + ">")
        , m_min_ui_height(min_ui_height) {
    }

    virtual ~DecoderBase() {}

    DecoderBase(DecoderBase&) = delete;
    DecoderBase& operator=(DecoderBase&) = delete;
    DecoderBase(DecoderBase&& other) {
        swap(*this, other);
    }

    friend void swap(DecoderBase& first, DecoderBase& second) {
        std::swap(first.m_name, second.m_name);
        std::swap(first.m_config_path, second.m_config_path);
        std::swap(first.m_input_sample_rate, second.m_input_sample_rate);
        std::swap(first.m_headless, second.m_headless);
        std::swap(first.m_log, second.m_log);
        std::swap(first.m_marker, second.m_marker);
        std::swap(first.m_center_frequency, second.m_center_frequency);
        std::swap(first.m_min_center_frequency, second.m_min_center_frequency);
        std::swap(first.m_min_ui_height, second.m_min_ui_height);
    }

    DecoderBase& operator=(DecoderBase&& other) = delete;

    void save_ui_settings();
    void load_ui_settings();
    std::string name() const { return m_name; }
    std::string config_path() const { return m_config_path; }
    u16 input_sample_rate() const { return m_input_sample_rate; }
    bool headless() const { return m_headless == Headless::Yes; }
    const Util::MarkerGroup& marker() const { return m_marker; }
    u16 center_frequency() const { return m_center_frequency; }
    u16 min_center_frequency() const { return m_min_center_frequency; }
    u16 min_ui_height() const { return m_min_ui_height; }
    void set_center_frequency(Hertz center_frequency);
    void set_status(const std::string&);

    virtual void setup() = 0;
    virtual void tear_down() = 0;
    virtual void process(float value) = 0;
    virtual Util::Buffer<std::string> changeable_parameters() const = 0;
    virtual bool setup_parameters(const Util::Buffer<std::string>&) = 0;
    virtual Pipe::GenericComponent& pipeline() = 0;

protected:
    const Logger& logger() const { return m_log; }
    void set_marker(Util::MarkerGroup);
    virtual void on_marker_move([[maybe_unused]] Hertz center_frequency) {}

private:
    std::string m_name;
    std::string m_config_path;
    SampleRate m_input_sample_rate { 0 };
    Headless m_headless { false };
    Logger m_log { "INVALID" };

    Util::MarkerGroup m_marker;
    Hertz m_center_frequency { 0 };
    Hertz m_min_center_frequency { 0 };
    u16 m_min_ui_height;
};

}
