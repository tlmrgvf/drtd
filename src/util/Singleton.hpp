#pragma once

#include <cassert>

namespace Util {

namespace SingletonAllocator {
template<typename T>
T* default_new() {
    return new T();
}

}

template<typename T, T* (*allocator_function)() = SingletonAllocator::default_new>
class Singleton final {
public:
    Singleton() = default;
    Singleton(Singleton&) = delete;
    Singleton(Singleton&&) = delete;

    ~Singleton() {
        if (m_instance)
            delete m_instance;
    }

    T* operator->() {
        return &operator*();
    }

    T& operator*() {
        if (!m_instance)
            m_instance = allocator_function();
        return *m_instance;
    }

    bool has_instance() const {
        return m_instance;
    }

private:
    T* m_instance { nullptr };
};

}

using Util::Singleton;
