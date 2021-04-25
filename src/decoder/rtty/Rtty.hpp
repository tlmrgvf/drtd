#pragma once

#include <Fl/Fl_Check_Button.H>
#include <decoder/Decoder.hpp>
#include <dsp/BitConverter.hpp>
#include <dsp/IQMixer.hpp>
#include <dsp/MovingAverage.hpp>
#include <dsp/Normalizer.hpp>
#include <ui/component/TextDisplay.hpp>
#include <ui/component/XYScope.hpp>
#include <util/BitBuffer.hpp>
#include <util/CallbackManager.hpp>
#include <util/Util.hpp>

namespace Dsp {

class Rtty final : public Decoder<bool> {
public:
    Rtty();

    virtual Util::Buffer<std::string> changeable_parameters() const override;
    virtual bool setup_parameters(const Util::Buffer<std::string>&) override;

protected:
    virtual Pipe::Line<float, bool> build_pipeline() override;
    Fl_Widget* build_ui(Util::Point, Util::Size) override;
    void process_pipeline_result(bool) override;
    virtual void on_setup() override;
    virtual void on_tear_down() override;
    virtual void on_marker_move(Hertz) override;

private:
    static constexpr SampleRate sample_rate { 7350 };
    static constexpr float scope_phase_step { Util::two_pi_f * (1000.f / sample_rate) };

    struct Settings {
        bool swap_mark_and_space { false };
        bool show_tuning { false };
        Hertz shift { 450 };
        float baud_rate { 45.45 };
    };

    void update_mixers();
    void update_marker();
    void update_filters();
    void update_scope(float, float);

    Settings m_settings;
    float m_scope_phase { 0 };
    bool m_wait_start { true };
    bool m_figures { false };
    ConfigRef<IQMixer> m_mark_mixer;
    ConfigRef<IQMixer> m_space_mixer;
    ConfigRef<Normalizer> m_mark_normalizer;
    ConfigRef<Normalizer> m_space_normalizer;
    ConfigRef<MovingAverageBase> m_mark_filter;
    ConfigRef<MovingAverageBase> m_space_filter;
    ConfigRef<BitConverter> m_converter;
    BitBuffer<Util::PushSequence::LsbPushedFirst, 7> m_input_buffer;
    Ui::TextDisplay* m_text_box { nullptr };
    Ui::XYScope* m_scope { nullptr };
    CallbackManager m_callback_manager;
};

}
