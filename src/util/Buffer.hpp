#pragma once

#include <cassert>
#include <initializer_list>
#include <memory>

namespace Util {

template<typename T>
class Buffer final {
public:
    Buffer()
        : m_values(nullptr)
        , m_size(0) {};

    Buffer(std::initializer_list<T>&& list)
        : Buffer(list.size()) {
        size_t count = 0;
        for (auto& item : list)
            operator[](count++) = std::move(item);
    }

    explicit Buffer(size_t size)
        : m_values(new T[size]())
        , m_size(size) {};

    Buffer(const Buffer& copy)
        : Buffer(copy.m_size) {
        std::copy(copy.m_values, copy.m_values + copy.m_size, m_values);
    };

    Buffer(Buffer&& move)
        : Buffer() {
        swap(*this, move);
    };

    ~Buffer() {
        if (m_values)
            delete[] m_values;
    }

    Buffer& operator=(const Buffer& copy) {
        Buffer tmp(copy);
        swap(*this, tmp);
        return *this;
    }

    Buffer& operator=(Buffer&& move) {
        swap(*this, move);
        return *this;
    }

    const T& operator[](size_t i) const {
        assert(i < m_size && m_values);
        return m_values[i];
    }

    T& operator[](size_t i) {
        assert(i < m_size && m_values);
        return m_values[i];
    }

    size_t size() const { return m_size; }
    T* ptr() { return m_values; }
    const T* ptr() const { return m_values; }
    bool operator==(const Buffer& compare) const { return compare.m_values == m_values && compare.m_size == m_size; }
    Buffer<T> resized(size_t new_size) const {
        Buffer<T> copy(new_size);

        size_t copy_size = std::min(new_size, m_size);
        std::copy(m_values, m_values + copy_size, copy.ptr());
        return copy;
    }

    class Iterator {
    public:
        Iterator(T* values, size_t index)
            : m_values(values)
            , m_index(index) {
        }

        Iterator& operator++() {
            ++m_index;
            return *this;
        }

        T& operator*() const {
            return m_values[m_index];
        }

        Iterator& operator+(size_t count) {
            m_index += count;
            return *this;
        }

        bool operator!=(const Iterator& compare) const {
            return !(m_values == compare.m_values) || m_index != compare.m_index;
        }

    private:
        T* m_values;
        size_t m_index;
    };

    Iterator begin() const { return Iterator(m_values, 0); }
    Iterator end() const { return Iterator(m_values, m_size); }

private:
    friend void swap(Buffer& one, Buffer& two) {
        std::swap(one.m_values, two.m_values);
        std::swap(one.m_size, two.m_size);
    }

    T* m_values;
    size_t m_size;
};

}

using Util::Buffer;
