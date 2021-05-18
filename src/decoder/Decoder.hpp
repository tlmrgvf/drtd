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

#include "DecoderBase.hpp"
#include <memory>
#include <pipe/Line.hpp>
#include <ui/MainGui.hpp>
#include <ui/component/Waterfall.hpp>

namespace Dsp {

template<typename T>
class IoComponent final : public Pipe::RefableComponent<T, T, IoComponent<T>> {
public:
    IoComponent(bool output)
        : Pipe::RefableComponent<T, T, IoComponent>("Input/Output")
        , m_output(output) {
    }

    virtual T process(T in) override { return in; }

    virtual Size calculate_size() override {
        return { 10, 10 };
    }

    virtual bool clicked_component(Point clicked_at, Pipe::ClickEvent event) override {
        if ((m_output && event == Pipe::ClickEvent::MonitorOutput) || (!m_output && event == Pipe::ClickEvent::MonitorInput))
            return false;

        bool result = Pipe::ComponentBase<T, T>::clicked_component(clicked_at, event);
        if (result && event == Pipe::ClickEvent::MonitorOutput && !m_output)
            Drtd::main_gui().waterfall().show_marker(true);
        return result;
    }

protected:
    virtual IoComponent& ref() override {
        return *this;
    }

    virtual void draw_at(Point p) override {
        auto size = calculate_size();
        fl_rect(p.x(), p.y(), size.w(), size.h());
        size.resize(-1, -1);
        fl_line(p.x(), p.y(), p.x() + size.w(), p.y() + size.h());
        fl_line(p.x(), p.y() + size.h(), p.x() + size.w(), p.y());
    }

private:
    bool m_output;
};

template<typename PipelineResult>
class Decoder : public DecoderBase {
public:
    Decoder(std::string name, SampleRate input_sample_rate, DecoderBase::Headless headless, u16 min_ui_height = 0)
        : DecoderBase(name, input_sample_rate, headless, min_ui_height) {
    }

    Decoder(Decoder&& other)
        : DecoderBase(std::move(other)) {
        std::swap(m_pipeline, other.m_pipeline);
    }

    Decoder<PipelineResult>& operator=(Decoder&& other) {
        std::swap(m_pipeline, other.m_pipeline);
    }

    virtual Pipe::GenericComponent& pipeline() final override {
        assert(m_pipeline);
        return *m_pipeline;
    }

    virtual void process(float value) final override {
        Pipe::GenericComponent::prepare_processing();
        PipelineResult result = m_pipeline->run(value);

        if (!Pipe::GenericComponent::did_abort_processing()) {
            Fl::lock();
            process_pipeline_result(result);
            Fl::awake();
            Fl::unlock();
        }
    }

    virtual void setup() final override {
        logger().info() << "setup()";

        auto input_component = IoComponent<float>(false);
        auto in_ref = input_component.make_ref();
        m_pipeline = std::make_unique<Pipe::Line<float, PipelineResult>>(
            Pipe::line(std::move(input_component), build_pipeline(), IoComponent<PipelineResult>(true)));

        int id = 0;
        m_pipeline->init(input_sample_rate(), id);
        on_setup();
        if (Drtd::using_ui()) {
            static constexpr u8 decoder_ui_padding = 8;

            in_ref->monitor(Pipe::Monitor::Output);
            auto& content_box = Drtd::main_gui().content_box();
            content_box.clear();
            auto* root = build_ui({ content_box.x() + decoder_ui_padding, content_box.y() + decoder_ui_padding },
                                  { static_cast<unsigned>(content_box.w() - 2 * decoder_ui_padding),
                                    static_cast<unsigned>(content_box.h() - 2 * decoder_ui_padding) });
            content_box.add(root);
            content_box.resizable(root);
            content_box.redraw();
        }
    }

    virtual void tear_down() final override {
        m_pipeline = nullptr;
        on_tear_down();
    }

    virtual Util::Buffer<std::string> changeable_parameters() const override {
        return {};
    }

    virtual bool setup_parameters(const Util::Buffer<std::string>&) override {
        return true;
    }

protected:
    virtual void on_tear_down() {};
    virtual Pipe::Line<float, PipelineResult> build_pipeline() = 0;
    virtual Fl_Widget* build_ui(Point top_left, Size ui_size) = 0;
    virtual void process_pipeline_result(PipelineResult) = 0;
    virtual void on_setup() {}

private:
    std::unique_ptr<Pipe::Line<float, PipelineResult>> m_pipeline;
};

}
