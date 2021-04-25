#include "Address.hpp"
#include <cassert>
#include <sstream>

using namespace Dsp::AX25Protocol;

Address::Address(Type type, std::string name, u8 ssid, u8 reserved, bool cbit, bool end_byte)
    : m_type(type)
    , m_name(std::move(name))
    , m_ssid(ssid)
    , m_reserved(reserved)
    , m_cbit(cbit)
    , m_end_byte(end_byte) {
}

Address::Address()
    : m_type(Type::Invalid)
    , m_name("INVALID")
    , m_ssid()
    , m_reserved()
    , m_cbit()
    , m_end_byte() {
}

std::optional<Address> Address::parse(Type type, const std::vector<u8>& bytes, unsigned offset) {
    if (bytes.size() <= offset + Address::address_block_size)
        return {};

    std::ostringstream builder;
    for (unsigned i = offset; i < offset + Address::address_block_size - 1; ++i) {
        builder << static_cast<char>(bytes[i] >> 1);
        if (mask_hdlc & bytes[i])
            return {};
    }

    u8 ssid = bytes[offset + Address::address_block_size - 1];
    return Address(type,
                   builder.str(),
                   ssid >> 1 & 0xF,
                   ssid >> 5 & 0x3,
                   ssid & mask_cbit,
                   ssid & mask_hdlc);
}

std::string Address::name_for_type(Type type) {
    switch (type) {
    case Type::Destination:
        return "Destination";
    case Type::Repeater:
        return "Repeater";
    case Type::Source:
        return "Source";
    case Type::Invalid:
        return "Invalid";
    }
    assert(false);
    return {};
}

bool Address::is_command() const {
    return (m_type == Type::Source && !m_cbit) || (m_type == Type::Destination && m_cbit);
}

bool Address::is_repeated() const {
    return m_type == Type::Repeater && m_cbit;
}

std::string Address::format() const {
    std::ostringstream builder;

    builder << "\"" << name() << "\"";
    if (is_repeated())
        builder << "[Rpt]";
    else if (is_command())
        builder << "[Cmd]";
    else
        builder << "     ";

    builder << std::hex << "(0x" << static_cast<unsigned>(ssid()) << ")";

    return builder.str();
}
