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

#include <util/Types.hpp>

namespace Dsp::PocsagProtocol {

class Data final {
public:
    static constexpr u8 codeword_bit_count { 32 };
    static constexpr u8 data_bit_count { 20 };
    static constexpr u16 preamble_bit_count { 576 };
    static constexpr u32 sync_word { 0x7CD215D8 };
    static constexpr u32 idle_word { 0x7A89C197 };
    static constexpr u32 data_mark_bit_mask { 0x100000 };
    static constexpr u32 data_msb_mask { 0x80000 };
    static constexpr u8 function_bits_mask { 3 };

    enum class Type : u8 {
        Address,
        Data,
        Idle
    };

    static Data from_codeword(u8 position_in_batch, u32 codeword);

    Type type() const { return m_type; }
    u8 function_bits() const { return m_function_bits; }
    u32 contents() const { return m_contents; }

private:
    Data(Type, u8 function_bits, u32 contents);

    Type m_type;
    u8 m_function_bits;
    u32 m_contents;
};

}
