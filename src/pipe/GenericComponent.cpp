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
#include "GenericComponent.hpp"
#include <Drtd.hpp>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <ui/MainGui.hpp>
#include <ui/component/Waterfall.hpp>
#include <util/Logger.hpp>
#include <util/Util.hpp>

using namespace Pipe;

static const Util::Logger s_log("ComponentBase");
Monitor GenericComponent::s_monitor { Monitor::Input };
int GenericComponent::s_monitor_id { -1 };
InterpreterProperties GenericComponent::s_interpreter { { "Nothing" }, 0xFF000000 };
u8 GenericComponent::s_interpreter_index { 0 };

static constexpr Point s_marker_monitor_input[] {
    { -14, -7 },
    { -6, -7 },
    { -2, -3 },
    { -2, -7 },
    { -2, -3 },
    { -6, -3 }
};

static constexpr Point s_marker_monitor_output[] {
    { 1, -7 },
    { 1, -3 },
    { 5, -3 },
    { 1, -3 },
    { 5, -7 },
    { 13, -7 }
};

bool GenericComponent::monitoring(int id, Monitor monitor) {
    return s_monitor_id == id && (s_monitor == monitor || monitor == Monitor::Either);
}

void GenericComponent::set_monitor(int id, Monitor monitor, InterpreterProperties properties) {
    assert(monitor != Monitor::Either);
    s_monitor_id = id;
    s_interpreter = properties;
    s_interpreter_index = std::min(s_interpreter_index, static_cast<u8>(properties.names.size() - 1));
    s_monitor = monitor;
}

void GenericComponent::draw(Point location) {
    m_absolute_position = location;
    fl_color(FL_BLACK);
    draw_at(location);
}

bool GenericComponent::clicked_component(Point clicked_at, ClickEvent event) {
    if (Util::rect_contains(clicked_at, m_absolute_position, this->calculate_size())) {
        auto& waterfall = Drtd::main_gui().waterfall();

        switch (event) {
        case ClickEvent::MonitorInput:
            monitor(Monitor::Input);
            waterfall.set_sample_rate(input_sample_rate());
            waterfall.show_marker(false);
            s_log.info() << "Now monitoring input at " << id();
            break;
        case ClickEvent::MonitorOutput:
            monitor(Monitor::Output);
            waterfall.set_sample_rate(output_sample_rate());
            waterfall.show_marker(false);
            s_log.info() << "Now monitoring output at " << id();
            break;
        case ClickEvent::Configure:
            show_config_dialog();
            break;
        default:
            assert(false);
        }

        return true;
    }

    return false;
}

void GenericComponent::show_config_dialog() {
    fl_message("Nothing to configure for \"%s\"", name().c_str());
}

void Pipe::draw_connecting_line_vertical(u32 from_x, u32 from_y, u32 to_y) {
    fl_yxline(from_x, from_y, to_y);
    fl_yxline(from_x + 1, from_y, to_y);
}

void Pipe::draw_connecting_line_horizontal(u32 from_x, u32 from_y, u32 to_x) {
    fl_xyline(from_x, from_y, to_x);
    fl_xyline(from_x, from_y + 1, to_x);
}

void Pipe::draw_marker(MarkerType type, Point pointing_at) {
    auto old_color = fl_color();
    fl_color(FL_DARK_GREEN);
    fl_push_matrix();
    fl_begin_line();

    if (type == MarkerType::MarkOutput) {
        fl_translate(pointing_at.x(), pointing_at.y());
        for (auto& point : s_marker_monitor_output)
            fl_vertex(point.x(), point.y());
    } else if (type == MarkerType::MarkInput) {
        fl_translate(pointing_at.x(), pointing_at.y());
        for (auto& point : s_marker_monitor_input)
            fl_vertex(point.x(), point.y());
    }

    fl_end_line();
    fl_pop_matrix();
    fl_color(old_color);
}

void Pipe::draw_simple_connector(Util::Point from, Util::Point to) {
    draw_connecting_line_horizontal(from.x(), from.y(), to.x());
}
