#pragma once

#include "Types.hpp"

namespace Util {

enum class PushSequence : bool {
    LsbPushedFirst,
    MsbPushedFirst
};

template<PushSequence sequence, u8 buffer_size>
class BitBuffer final {
public:
    using BufferType = u64;
    static_assert(buffer_size > 0 && buffer_size < sizeof(BufferType) * 8, "Bad size");
    static constexpr BufferType all_ones { ~static_cast<BufferType>(0) };
    static constexpr BufferType result_mask { ~(all_ones << buffer_size) };
    static constexpr BufferType shifted_out_bit_mask { static_cast<BufferType>(1) << (buffer_size - 1) };

    bool get(u8 bit) const { return ((1 << bit) & m_buffer); }

    bool push(bool bit) {
        BufferType old;

        if constexpr (sequence == PushSequence::LsbPushedFirst) {
            BufferType input_bit = bit;
            input_bit <<= buffer_size - 1;
            old = m_buffer & 1;
            m_buffer >>= 1;
            m_buffer = (m_buffer & ~shifted_out_bit_mask) | input_bit;
        } else if (sequence == PushSequence::MsbPushedFirst) {
            old = m_buffer & shifted_out_bit_mask;
            m_buffer = ((m_buffer << 1) & result_mask) | static_cast<BufferType>(bit);
        }

        m_bit_counter = static_cast<u8>((m_bit_counter + 1) % buffer_size);
        return old;
    }

    void reset_bit_count() { m_bit_counter = 0; }

    void reset() {
        m_bit_counter = 0;
        m_buffer = 0;
    }

    u8 bit_count() const { return m_bit_counter; }
    bool aligned() const { return m_bit_counter == 0; }

    template<typename T>
    T data() const {
        static_assert(sizeof(T) * 8 >= buffer_size, "Data type too small");
        return static_cast<T>(m_buffer & result_mask);
    }

private:
    u8 m_bit_counter { 0 };
    BufferType m_buffer { 0 };
};

}

using Util::BitBuffer;
