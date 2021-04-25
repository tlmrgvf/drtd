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
