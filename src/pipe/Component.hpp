#pragma once

#include "GenericComponent.hpp"
#include <Drtd.hpp>
#include <memory>
#include <string>
#include <util/Util.hpp>

namespace Pipe {

template<typename In, typename Out>
class ComponentBase : public GenericComponent {
public:
    using InputType = In;
    using OutputType = Out;

    ComponentBase(std::string name)
        : GenericComponent(std::move(name)) {}
    ComponentBase(ComponentBase&& other)
        : GenericComponent(std::move(other)) {}

    Out run(In in) {
        if (GenericComponent::s_abort_processing)
            return {};

        Out result = process(in);
        if (GenericComponent::s_monitor_id == id() && id() >= 0) {
            if (GenericComponent::s_monitor == Monitor::Input)
                Drtd::monitor_sample(m_in_interpreter.interpreter_function(GenericComponent::s_interpreter_index, in));
            else if (!GenericComponent::did_abort_processing())
                Drtd::monitor_sample(m_out_interpreter.interpreter_function(GenericComponent::s_interpreter_index, result));
        }

        return result;
    }

    template<typename I, typename M, typename O>
    friend ComponentBase<M, O>& operator>>(ComponentBase<I, M>& left, ComponentBase<M, O>& right);

    virtual InterpreterProperties properties(Monitor mon) const final override {
        if (mon == Monitor::Input)
            return m_in_interpreter;
        else if (mon == Monitor::Output)
            return m_out_interpreter;

        assert(false);
        return {};
    }

    virtual SampleRate init(SampleRate input_sample_rate, int& component_id) final override {
        set_id(component_id++);
        logger().info() << "Init component";
        set_input_sample_rate(input_sample_rate);
        set_output_sample_rate(on_init(input_sample_rate, component_id));
        assert(output_sample_rate());
        logger().info() << "Input S/R " << input_sample_rate << "Hz, output S/R " << output_sample_rate() << "Hz, id " << id();

        return output_sample_rate();
    }

    In& input_buffer() { return m_input_buffer; }
    void set_input_buffer(In in) { m_input_buffer = in; }

protected:
    virtual SampleRate on_init(SampleRate input_sample_rate, [[maybe_unused]] int& component_id) { return input_sample_rate; }
    virtual Out process(In) = 0;

private:
    Interpreter<In> m_in_interpreter { interpreter<In>() };
    Interpreter<Out> m_out_interpreter { interpreter<Out>() };
    In m_input_buffer;
};

template<typename I, typename M, typename O>
static ComponentBase<M, O>& operator>>(ComponentBase<I, M>& left, ComponentBase<M, O>& right) {
    right.m_input_buffer = left.run(left.m_input_buffer);
    return right;
}

template<typename RefType>
class RefableBase {
public:
    template<typename>
    friend class ConfigRef;
    virtual ~RefableBase() = default;

protected:
    virtual RefType& ref() = 0;
};

template<typename RefType>
class ConfigRef final {
public:
    template<typename, typename, typename>
    friend class RefableComponent;
    ConfigRef() = default;

    RefType* operator->() {
        return &operator*();
    }

    RefType& operator*() {
        assert(valid());
        return (*m_component_ptr)->ref();
    }

    bool valid() const {
        return m_component_ptr != nullptr && *m_component_ptr != nullptr;
    }

private:
    explicit ConfigRef(std::shared_ptr<RefableBase<RefType>*> ptr)
        : m_component_ptr(ptr) {
    }

    std::shared_ptr<RefableBase<RefType>*> m_component_ptr;
};

template<typename In, typename Out, typename RefType>
class RefableComponent : public ComponentBase<In, Out>
    , public RefableBase<RefType> {
public:
    explicit RefableComponent(std::string name)
        : ComponentBase<In, Out>(std::move(name))
        , m_component_ptr(std::make_shared<RefableBase<RefType>*>(this)) {}

    RefableComponent(const RefableComponent&) = delete;
    RefableComponent(RefableComponent&& other)
        : ComponentBase<In, Out>(std::move(other))
        , m_component_ptr(std::make_shared<RefableBase<RefType>*>(this)) {
        swap(*this, other);
    }

    virtual ~RefableComponent() {
        if (m_component_ptr.use_count() > 1)
            this->logger().warning() << "Destroying component with references!";
        *m_component_ptr = nullptr;
    }

    RefableComponent& operator=(RefableComponent&& to_move) {
        swap(*this, to_move);
        return *this;
    }

    friend void swap(RefableComponent& first, RefableComponent& second) {
        std::swap(first.m_component_ptr, second.m_component_ptr);
        std::swap(*first.m_component_ptr, *second.m_component_ptr);
    }

    ConfigRef<RefType> make_ref() { return ConfigRef<RefType>(m_component_ptr); }

private:
    std::shared_ptr<RefableBase<RefType>*> m_component_ptr;
};

template<typename... Components>
struct LastOutputOf {
    using OutputType = typename decltype(
        (Util::TypeIdentity<Components>(), ...))::Type::OutputType;
};

template<typename First, typename...>
struct FirstComponent {
    using OutputType = typename First::OutputType;
    using InputType = typename First::InputType;
};

}

using Pipe::ComponentBase;
using Pipe::ConfigRef;
using Pipe::RefableComponent;
