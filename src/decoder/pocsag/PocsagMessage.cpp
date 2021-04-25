#include "PocsagMessage.hpp"
#include <cassert>
#include <util/Util.hpp>

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

    builder << "\n";
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
