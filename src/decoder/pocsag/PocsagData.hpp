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
