#pragma once

#include <functional>
#include <memory>
#include <pipe/GenericComponent.hpp>
#include <util/Util.hpp>

namespace Pipe::Container {
template<typename In, typename Out>
struct ComponentContainerBase {
    ComponentContainerBase() {}
    virtual ~ComponentContainerBase() {}
    ComponentContainerBase(const ComponentContainerBase&) = delete;

    virtual Out run(In) = 0;
    virtual void for_each(std::function<Util::IterationDecision(GenericComponent&)>) = 0;
    virtual GenericComponent& first() = 0;
    virtual GenericComponent& last() = 0;
    virtual size_t size() = 0;
};
}
