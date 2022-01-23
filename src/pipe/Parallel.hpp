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
#include "Component.hpp"
#include <FL/fl_draw.H>
#include <functional>
#include <list>
#include <pipe/ComponentContainer.hpp>
#include <util/Buffer.hpp>

namespace Pipe {

template<typename In, typename Out, typename... Lines>
class ParallelContainer final : public Container::ComponentContainerBase<In, const Util::Buffer<Out>&> {
public:
    using Iterator = std::function<Util::IterationDecision(GenericComponent&)>;
    using OutputBuffer = Util::Buffer<Out>;

    ParallelContainer(Lines&&... lines)
        : m_lines(std::make_tuple(std::move(lines)...))
        , m_output_buffer(sizeof...(Lines)) {
        static_assert(sizeof...(lines) > 0);
    }

    virtual const OutputBuffer& run(In in) override {
        std::apply([&](Lines&... lines) { ParallelContainer::run(m_output_buffer, 0, in, lines...); }, m_lines);
        return m_output_buffer;
    }

    virtual void for_each(Iterator callback) override {
        std::apply([&](Lines&... lines) { ParallelContainer::for_each_component(callback, lines...); }, m_lines);
    }

    virtual GenericComponent& first() override {
        return std::get<0>(m_lines);
    }

    virtual GenericComponent& last() override {
        return std::get<sizeof...(Lines) - 1>(m_lines);
    }

    virtual size_t size() override {
        return sizeof...(Lines);
    }

private:
    template<typename... Components>
    void run(OutputBuffer& buffer, size_t index, In in, ComponentBase<In, Out>& component, Components&... others) {
        buffer[index] = component.run(in);
        run(buffer, index + 1, in, others...);
    }

    static void run(OutputBuffer&, size_t, In) {
    }

    template<typename... Components>
    void for_each_component(Iterator& callback, GenericComponent& component, Components&... others) {
        if (callback(component) == Util::IterationDecision::Continue)
            (for_each_component(callback, others), ...);
    }

    static void for_each_component(Iterator&) {
    }

    std::tuple<Lines...> m_lines;
    OutputBuffer m_output_buffer;
};

static constexpr Util::Size marker_size = { 14, 14 };
static constexpr u8 pipeline_horizontal_spacing = 16;
static constexpr u8 pipeline_horizontal_spacing_adjusted = pipeline_horizontal_spacing - (marker_size.w() / 2);

template<typename In, typename Out, typename MergeOut>
class Parallel final : public ComponentBase<In, MergeOut> {
public:
    using ContainerOut = const Util::Buffer<Out>&;
    using ContainerType = Container::ComponentContainerBase<In, ContainerOut>;
    Parallel(std::function<MergeOut(ContainerOut)> merge_function, std::unique_ptr<ContainerType> lines)
        : ComponentBase<In, MergeOut>("Parallel")
        , m_merge_function(merge_function)
        , m_lines(std::move(lines)) {
    }

    virtual Size calculate_size() override {
        unsigned height_sum = 0;
        unsigned max_width = 0;

        m_lines->for_each([&](GenericComponent& line) {
            auto size = line.calculate_size();
            max_width = std::max(size.w(), max_width);
            height_sum += size.h() + Pipe::component_vertical_spacing;
            return Util::IterationDecision::Continue;
        });

        height_sum -= Pipe::component_vertical_spacing;
        return { max_width + pipeline_horizontal_spacing + pipeline_horizontal_spacing_adjusted, height_sum };
    }

    virtual bool clicked_component(Point clicked_at, ClickEvent event) override {
        bool hit = false;

        m_lines->for_each([&](auto& line) {
            if (line.clicked_component(clicked_at, event)) {
                hit = true;
                return Util::IterationDecision::Break;
            }

            return Util::IterationDecision::Continue;
        });

        if (hit)
            return true;

        if (Util::rect_contains(clicked_at, m_marker_location, marker_size) && event == ClickEvent::MonitorOutput) {
            GenericComponent::monitor(Monitor::Output);
            return true;
        }

        return false;
    }

protected:
    virtual void draw_at(Point position) override {
        auto abs_position = position;
        position.translate(pipeline_horizontal_spacing_adjusted, 0);

        int marker_index = -1;
        Buffer<Point> left_points(m_lines->size());
        Buffer<Point> right_points(m_lines->size());

        int point_index = 0;
        m_lines->for_each([&](GenericComponent& line) {
            if (GenericComponent::monitoring(line.id(), Monitor::Either))
                marker_index = point_index;

            auto line_size = line.calculate_size();
            line.draw(position);

            left_points[point_index] = { position.x() - pipeline_horizontal_spacing_adjusted, static_cast<int>(position.y() + line_size.h() / 2 - 1) };
            right_points[point_index] = { static_cast<int>(position.x() + line_size.w()), static_cast<int>(position.y() + line_size.h() / 2 - 1) };
            position.translate(0, line_size.h() + Pipe::component_vertical_spacing);
            ++point_index;
            return Util::IterationDecision::Continue;
        });

        auto& some_line = m_lines->first();
        fl_color(some_line.properties(Monitor::Input).color);
        draw_opening_connectors(left_points, marker_index);
        draw_closing_connectors(abs_position, right_points, some_line.properties(Monitor::Output).color, marker_index);
    }

    virtual SampleRate on_init(SampleRate input_sample_rate, int& id_counter) override {
        SampleRate output = 0;
        m_lines->for_each([&](auto& component) {
            auto line_rate = component.init(input_sample_rate, id_counter);
            if (!output)
                output = line_rate;
            else if (line_rate != output)
                Util::die("Output sample rate differs in parallel pipeline!");
            return Util::IterationDecision::Continue;
        });
        return output;
        return input_sample_rate;
    }

    MergeOut process(In in) override {
        auto& result = m_lines->run(in);
        if (GenericComponent::did_abort_processing())
            return {};

        return m_merge_function(result);
    }

private:
    void draw_opening_connectors(const Util::Buffer<Util::Point>& source_points, int marker_index) {
        u32 x_target = source_points[0].x() + pipeline_horizontal_spacing_adjusted - 1;
        auto direction = GenericComponent::current_monitor() == Monitor::Input ? MarkerType::MarkInput : MarkerType::NoMarker;

        int min_y = std::numeric_limits<int>::max();
        int max_y = std::numeric_limits<int>::min();
        for (auto& src : source_points) {
            min_y = std::min(min_y, src.y());
            max_y = std::max(max_y, src.y());
        }

        Pipe::draw_connecting_line_vertical(source_points[0].x(), min_y, max_y);
        int index = 0;
        for (auto& src : source_points) {
            Point target(x_target, src.y());
            if (index == marker_index)
                Pipe::draw_marker(direction, target);
            Pipe::draw_simple_connector(src, target);
            ++index;
        }
    }

    void draw_closing_connectors(const Util::Point& absolute_position, const Util::Buffer<Util::Point>& source_points, Fl_Color intermediate, int marker_index) {
        int min_y = std::numeric_limits<int>::max();
        int max_y = std::numeric_limits<int>::min();
        auto position = GenericComponent::current_monitor() == Monitor::Output ? MarkerType::MarkOutput : MarkerType::NoMarker;
        int x_target = std::numeric_limits<int>::min();

        fl_color(intermediate);
        for (auto& src : source_points) {
            x_target = std::max(x_target, src.x());
            min_y = std::min(min_y, src.y());
            max_y = std::max(max_y, src.y());
        }
        x_target += pipeline_horizontal_spacing_adjusted;

        Pipe::draw_connecting_line_vertical(x_target, min_y, max_y + 1);
        int index = 0;
        for (auto& src : source_points) {
            if (index == marker_index)
                Pipe::draw_marker(position, src);

            Pipe::draw_simple_connector(src, { x_target + 1, src.y() });
            ++index;
        }

        fl_color(FL_BLACK);
        m_marker_location = { x_target + 1, static_cast<int>(absolute_position.y() + calculate_size().h() / 2) };
        fl_polygon(m_marker_location.x() - 6, m_marker_location.y() + 6,
                   m_marker_location.x() - 6, m_marker_location.y() - 7,
                   m_marker_location.x() + 6, m_marker_location.y() - 1,
                   m_marker_location.x() + 6, m_marker_location.y());
        m_marker_location.translate(-static_cast<int>(marker_size.w() / 2), -static_cast<int>(marker_size.h() / 2));
    }

    Util::Point m_marker_location;
    std::function<MergeOut(const Util::Buffer<Out>&)> m_merge_function;
    std::unique_ptr<ContainerType> m_lines;
};

template<typename MergeOut, typename... Lines, typename In = typename FirstComponent<Lines...>::InputType, typename Out = typename FirstComponent<Lines...>::OutputType>
static Parallel<In, Out, MergeOut> parallel(std::function<MergeOut(const Buffer<Out>&)> merge_func, Lines&&... lines) {
    return Parallel<In, Out, MergeOut>(merge_func, std::make_unique<ParallelContainer<In, Out, Lines...>>(std::move(lines)...));
}

}
