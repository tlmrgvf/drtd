#include "PocsagData.hpp"

using namespace Dsp::PocsagProtocol;

Data::Data(Type type, u8 function_bits, u32 contents)
    : m_type(type)
    , m_function_bits(function_bits)
    , m_contents(contents) {
}

Data Data::from_codeword(u8 position_in_batch, u32 codeword) {
    if (codeword == idle_word >> 1) //Remove parity
        return Data(Type::Idle, 0, 0);

    codeword >>= 10; //Remove BCH code bits
    if (codeword & data_mark_bit_mask)
        return Data(Type::Data, 0, codeword & ~data_mark_bit_mask);
    else
        return Data(Type::Address, static_cast<u8>(codeword & function_bits_mask), ((codeword & ~function_bits_mask) << 1) | position_in_batch / 2);
}
