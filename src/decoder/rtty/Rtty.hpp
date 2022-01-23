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
#pragma once

#include <FL/Fl_Check_Button.H>
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
#include <util/SNRCalculator.hpp>

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

    Util::SNRCalculator m_mark_snr;
    Util::SNRCalculator m_space_snr;
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
