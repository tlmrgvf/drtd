#pragma once

#include "Buffer.hpp"

namespace Util {

template<typename T>
class RingBuffer final {
public:
    RingBuffer()
        : RingBuffer(0) {
    }

    RingBuffer(size_t size)
        : m_buffer(size)
        , m_size(size) {
        clear();
    }

    void clear() {
        for (size_t i = 0; i < m_buffer.size(); ++i)
            m_buffer[i] = T();

        m_next_empty = 0;
        m_count = 0;
        m_next_read = 0;
    }

    void resize(size_t size) {
        if (size == m_size) {
            clear();
            return;
        }

        m_size = size;
        m_buffer = Buffer<T>(size);
        clear();
    }

    size_t size() const { return m_size; }
    const T& peek(size_t index) const { return m_buffer[(index + m_next_empty) % m_size]; }
    T& peek(size_t index) { return m_buffer[(index + m_next_empty) % m_size]; }
    bool is_full() const { return m_count == m_size; }
    size_t count() const { return m_count; }
    size_t next_empty() const { return m_next_empty; }
    T pop() {
        assert(m_count);

        T& value = m_buffer[m_next_read];
        m_next_read = (m_next_read + 1) % m_size;
        --m_count;

        return value;
    }

    T push(T value) {
        T removed = m_buffer[m_next_empty];
        m_buffer[m_next_empty] = std::move(value);
        m_next_empty = (m_next_empty + 1) % m_size;
        m_count = std::min(m_count + 1, m_size);
        return removed;
    }

private:
    Buffer<T> m_buffer;
    size_t m_size;
    size_t m_next_empty { 0 };
    size_t m_count { 0 };
    size_t m_next_read { 0 };
};

}

using Util::RingBuffer;
