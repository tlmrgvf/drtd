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

#include <Fl/fl_draw.H>
#include <pipe/Component.hpp>
#include <pipe/ComponentContainer.hpp>

namespace Pipe {

template<typename In, typename Out, typename... Components>
class LineContainer final : public Container::ComponentContainerBase<In, Out> {
public:
    using Iterator = std::function<Util::IterationDecision(GenericComponent&)>;

    LineContainer(std::tuple<Components...> components)
        : m_components(std::move(components)) {}

    virtual Out run(In in) override {
        std::get<0>(m_components).set_input_buffer(in);
        return std::apply(
            [&](Components&... components) -> Out {
                auto& last = (... >> components);
                return last.run(last.input_buffer());
            },
            m_components);
    }

    virtual void for_each(Iterator callback) override {
        std::apply(
            [&](Components&... components) {
                LineContainer::for_each_component(callback, components...);
            },
            m_components);
    }

    virtual GenericComponent& first() override {
        return std::get<0>(m_components);
    }

    virtual GenericComponent& last() override {
        return std::get<sizeof...(Components) - 1>(m_components);
    }

    virtual size_t size() override { return sizeof...(Components); }

private:
    template<typename... IterateComponents>
    void for_each_component(Iterator& callback, GenericComponent& component,
                            IterateComponents&... others) {
        if (callback(component) == Util::IterationDecision::Continue)
            (for_each_component(callback, others), ...);
    }

    static void for_each_component(Iterator&) {}

    std::tuple<Components...> m_components;
};

template<typename In, typename Out>
class Line final : public ComponentBase<In, Out> {
public:
    Line(std::unique_ptr<Container::ComponentContainerBase<In, Out>> container)
        : ComponentBase<In, Out>("Line")
        , m_components(std::move(container)) {
        assert(m_components);
    }

    virtual Util::Size calculate_size() override {
        unsigned max_height = 0;
        unsigned width_sum = 0;

        m_components->for_each([&](GenericComponent& component) {
            auto size = component.calculate_size();
            max_height = std::max(size.h(), max_height);
            width_sum += size.w() + Pipe::component_horizontal_spacing;
            return Util::IterationDecision::Continue;
        });

        width_sum -= Pipe::component_horizontal_spacing;
        return { width_sum, max_height };
    }

    virtual bool clicked_component(Util::Point clicked_at, ClickEvent event) override {
        bool click_handled = false;
        m_components->for_each([&](GenericComponent& component) {
            if (component.clicked_component(clicked_at, event)) {
                click_handled = true;
                return Util::IterationDecision::Break;
            }
            return Util::IterationDecision::Continue;
        });

        return click_handled;
    }

protected:
    virtual void draw_at(Util::Point position) override {
        auto size = calculate_size();
        unsigned center = size.h() / 2;
        position.translate(0, center);
        auto last_component = &m_components->last();

        m_components->for_each([&](GenericComponent& component) {
            auto component_size = component.calculate_size();

            Point component_right = position.translated(component_size.w(), 0);
            component.draw({ position.x(), static_cast<int>(position.y() - component_size.h() / 2) });

            if (GenericComponent::monitoring(component.id(), Monitor::Input))
                Pipe::draw_marker(MarkerType::MarkInput, position);
            else if (GenericComponent::monitoring(component.id(), Monitor::Output))
                Pipe::draw_marker(MarkerType::MarkOutput, component_right);

            if (last_component != &component) {
                fl_color(component.properties(Monitor::Output).color);
                Pipe::draw_simple_connector(component_right.translated(0, -1), component_right.translated(Pipe::component_horizontal_spacing - 1, -1));
            }

            position = component_right.translated(Pipe::component_horizontal_spacing, 0);
            return Util::IterationDecision::Continue;
        });
    }

    virtual SampleRate on_init(SampleRate input_sample_rate, int& id_counter) override {
        SampleRate output_rate = input_sample_rate;
        m_components->for_each([&](GenericComponent& comp) {
            output_rate = comp.init(output_rate, id_counter);
            return Util::IterationDecision::Continue;
        });
        return output_rate;
    }

    Out process(In in) override { return m_components->run(in); }

private:
    std::unique_ptr<Container::ComponentContainerBase<In, Out>> m_components;
};

template<typename... Others,
         typename In = typename FirstComponent<Others...>::InputType,
         typename Out = typename LastOutputOf<Others...>::OutputType>
static Line<In, Out> line(Others&&... others) {
    auto container = std::make_unique<LineContainer<In, Out, Others...>>(std::make_tuple(std::move(others)...));
    return Line<In, Out>(std::move(container));
}

}
