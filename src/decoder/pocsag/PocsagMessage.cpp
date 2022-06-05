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
#include "PocsagMessage.hpp"
#include <cassert>
#include <chrono>
#include <util/Util.hpp>
#include <array>

using namespace Dsp::PocsagProtocol;

/* https://www.raveon.com/pdfiles/AN142(POCSAG).pdf */
static constexpr std::array numeric_code_map { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', 'U', ' ', '-', ')', '(' };

Message::Message(ContentType type,
                 std::optional<Data> address,
                 std::string alpha,
                 std::string numeric,
                 bool has_invalid_code_word,
                 bool has_data,
                 u16 baud_rate)
    : m_type(type)
    , m_address(address)
    , m_alpha(alpha)
    , m_numeric(numeric)
    , m_has_invalid_code_word(has_invalid_code_word)
    , m_has_data(has_data)
    , m_baud_rate(baud_rate) {
}

std::string Message::content_name(ContentType type) {
    switch (type) {
    case ContentType::NoContent:
        return "None";
    case ContentType::AlphaNumeric:
        return "Alpha numeric";
    case ContentType::Numeric:
        return "Numeric";
    case ContentType::Both:
        return "Both";
    default:
        assert(false);
        return "";
    }
}

std::string Message::str() const {
    std::ostringstream builder;
    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    builder << "Received at " << std::ctime(&time);
    builder << "POCSAG" << m_baud_rate << " | Address: ";
    if (m_address.has_value())
        builder << m_address.value().contents();
    else
        builder << "-";

    builder << " | Function: ";
    if (m_address.has_value())
        builder << static_cast<int>(m_address.value().function_bits());
    else
        builder << "-";

    if (m_has_invalid_code_word)
        builder << " | Errors detected!";

    if (m_has_data) {
        if (m_type == ContentType::AlphaNumeric || m_type == ContentType::Both)
            builder << "\n\tAlphanumeric: " << m_alpha;

        if (m_type == ContentType::Numeric || m_type == ContentType::Both)
            builder << "\n\tNumeric: " << m_numeric;
    } else {
        builder << " (No data)";
    }

    builder << "\n\n";
    return builder.str();
}

void MessageBuilder::append_data(const Data& data) {
    if (data.type() == Data::Type::Address) {
        m_address = data;
        m_valid = true;
        return;
    } else if (data.type() == Data::Type::Idle) {
        return;
    }

    m_has_data = true;
    m_valid = true;
    auto contents = data.contents();

    /* Parse as alpha-numeric message */
    for (u8 i = 0; i < Data::data_bit_count && contents; ++i) {
        m_raw_data_bits >>= 1;

        if (contents & Data::data_msb_mask)
            m_raw_data_bits |= 0x40;

        if (m_raw_bit_count >= 6) {
            m_alpha_builder << Util::escape_ascii(m_raw_data_bits);
            m_raw_bit_count = 0;
            m_raw_data_bits = 0;
        } else {
            ++m_raw_bit_count;
        }

        contents <<= 1;
    }

    /* Parse as numeric message */
    contents = data.contents();
    for (i8 i = Data::data_bit_count - 4; i >= 0; i -= 4)
        m_numerical_builder << numeric_code_map[(contents >> i) & 0xF];
}

Message MessageBuilder::build(Message::ContentType type, BaudRate rate) {
    return Message(type, m_address, m_alpha_builder.str(), m_numerical_builder.str(), m_has_invalid_code_word, m_has_data, rate);
}
