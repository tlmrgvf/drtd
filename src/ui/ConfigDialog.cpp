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
#include "ConfigDialog.hpp"
#include <Drtd.hpp>
#include <FL/fl_ask.H>
#include <decoder/Decoder.hpp>
#include <pipe/Component.hpp>
#include <util/Buffer.hpp>
#include <util/Logger.hpp>
#include <util/Singleton.hpp>
#include <util/Util.hpp>

using namespace Ui;

Singleton<ConfigDialog> s_config_dialog;
const Util::Logger s_log("ConfigDialog");
constexpr u16 min_width { 250 };
constexpr u16 min_height { 180 };
constexpr u16 pipeline_padding { 10 };
constexpr u16 combo_box_width { 60 };
constexpr u16 combo_box_offset { 70 };

ConfigDialog::ConfigDialog()
    : Fl_Window(0, 0, min_width, min_height, "Configure")
    , m_audio_line(combo_box_offset, 4, w() - 4 - combo_box_offset, 25, "Input:")
    , m_monitored_value(m_audio_line.x(), m_audio_line.y() + m_audio_line.h() + 4, m_audio_line.w(), m_audio_line.h(), "Monitor:")
    , m_pipeline_canvas(4, m_monitored_value.y() + m_monitored_value.h() + 4, w() - 8, h() - m_monitored_value.y() - m_monitored_value.h() - 36, 2)
    , m_pipeline_layer(m_pipeline_canvas.make_layer(0, 0, Layer::parent_size, Layer::parent_size, color(), true))
    , m_reset_button(m_pipeline_canvas.x(), m_pipeline_canvas.y() + m_pipeline_canvas.h() + 4, w() - 8, 24, "Reset pipeline") {
    m_audio_line.align(FL_ALIGN_LEFT);

    if (Drtd::audio_lines().size()) {
        for (auto& line : Drtd::audio_lines())
            m_audio_line.add(line.name.c_str());
    } else {
        m_audio_line.add("stdin");
        m_audio_line.deactivate();
    }

    m_audio_line.callback([](auto, auto*) {
        auto old_line = Drtd::current_audio_line();
        u8 new_line = static_cast<u8>(s_config_dialog->m_audio_line.value());
        if (!Drtd::switch_audio_line(new_line)) {
            if (!Drtd::switch_audio_line(old_line))
                Util::die("Can't use previously used input line! Outdated config file? Invalid input?");
            fl_alert("Could not use audio line \"%s\"!", Drtd::audio_lines()[new_line].name.c_str());
            s_config_dialog->m_audio_line.value(old_line);
        }
    });

    m_monitored_value.callback([](auto, auto*) {
        Pipe::GenericComponent::set_interpreter_index(static_cast<u8>(s_config_dialog->m_monitored_value.value()));
    });

    m_pipeline_canvas.box(FL_DOWN_BOX);
    m_pipeline_canvas.m_on_click = [&](auto point, int button, bool ctrl) { pipeline_click_event(point, button, ctrl); };
    m_pipeline_canvas.tooltip("Right click: Configure\nCtrl & left/right click: Monitor input/output");
    m_reset_button.callback([](auto, auto*) {
        Drtd::stop_processing();
        Drtd::start_processing(Drtd::active_decoder_index());
    });
}

void ConfigDialog::pipeline_click_event(Util::Point point, int button, bool ctrl) {
    s_log.info() << "Click at " << point;
    auto event = Pipe::ClickEvent::Invalid;

    if (button == 3) {
        /* Right click */
        if (ctrl)
            event = Pipe::ClickEvent::MonitorOutput;
        else
            event = Pipe::ClickEvent::Configure;
    } else if (button == 1 && ctrl) {
        /* Left click */
        event = Pipe::ClickEvent::MonitorInput;
    }

    if (event != Pipe::ClickEvent::Invalid) {
        if (Drtd::active_decoder()->pipeline().clicked_component(point, event))
            update(false);
    }
}

void ConfigDialog::show_dialog() {
    auto& main_gui = Drtd::main_gui();

    s_config_dialog->position(
        main_gui.x() + Util::center(main_gui.w(), s_config_dialog->w()),
        main_gui.y() + Util::center(main_gui.h(), s_config_dialog->h()));

    s_config_dialog->set_non_modal();
    s_config_dialog->show();
    s_config_dialog->update(true);
}

void ConfigDialog::update(bool reposition) {
    m_audio_line.value(Drtd::current_audio_line());
    auto& line = Drtd::active_decoder()->pipeline();
    auto size = line.calculate_size();
    auto& interpreter = Pipe::GenericComponent::current_interpreter();

    m_monitored_value.clear();
    for (auto& name : interpreter.names)
        m_monitored_value.add(name.c_str());
    m_monitored_value.value(Pipe::GenericComponent::interpreter_index());

    //FIXME: This resizing is *a bit* ugly
    auto& main_gui = Drtd::main_gui();
    u16 new_width = std::max(min_width, static_cast<u16>(pipeline_padding + size.w() + 2 * m_pipeline_canvas.padding() + 8));
    u16 new_height = std::max(min_height, static_cast<u16>(pipeline_padding + size.h() + m_pipeline_canvas.y() + 2 * m_pipeline_canvas.padding() + 12 + m_reset_button.h()));
    int new_x = reposition ? main_gui.x() + Util::center(main_gui.w(), new_width) : x();
    int new_y = reposition ? main_gui.y() + Util::center(main_gui.h(), new_height) : y();

    m_audio_line.resize(m_audio_line.x(), m_audio_line.y(), new_width - 4 - combo_box_offset, m_audio_line.h());
    m_monitored_value.resize(m_monitored_value.x(), m_monitored_value.y(), m_audio_line.w(), m_audio_line.h());
    m_reset_button.resize(m_reset_button.x(), m_reset_button.y(), new_width - 8, m_reset_button.h());
    resize(new_x, new_y, new_width, new_height);
    m_pipeline_canvas.resize(m_pipeline_canvas.x(),
                             m_pipeline_canvas.y(),
                             new_width - 8,
                             new_height - m_audio_line.y() - m_audio_line.h() - m_monitored_value.h() - 38);
    m_pipeline_canvas.resize_offscreen_buffers();
    m_reset_button.position(m_pipeline_canvas.x(), m_pipeline_canvas.y() + m_pipeline_canvas.h() + 2);

    {
        Ui::LayerDraw draw(m_pipeline_layer);
        fl_rectf(0, 0, m_pipeline_layer->current_width(), m_pipeline_layer->current_height(), m_pipeline_layer->clear_color());
        fl_color(FL_BLACK);
        line.draw({ Util::center(m_pipeline_layer->current_width(), size.w()), Util::center(m_pipeline_layer->current_height(), size.h()) });
    }

    m_pipeline_canvas.damage(FL_DAMAGE_ALL);
}

void ConfigDialog::refresh() {
    if (s_config_dialog.has_instance())
        s_config_dialog->update(false);
}
