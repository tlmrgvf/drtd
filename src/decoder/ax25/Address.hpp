#pragma once

#include <optional>
#include <string>
#include <util/Types.hpp>
#include <vector>

namespace Dsp::AX25Protocol {

class Address {
public:
    enum class Type : u8 {
        Destination,
        Source,
        Repeater,
        Invalid
    };

    static constexpr u8 address_block_size = 7;
    static std::optional<Address> parse(Type, const std::vector<u8>& bytes, unsigned offset);
    static std::string name_for_type(Type);

    Address();

    std::string format() const;
    Type type() const { return m_type; }
    const std::string& name() const { return m_name; }
    u8 ssid() const { return m_ssid; }
    u8 reserved() const { return m_reserved; }
    bool cbit() const { return m_cbit; }
    bool is_end_byte() const { return m_end_byte; }
    bool is_repeated() const;
    bool is_command() const;

private:
    static constexpr u8 mask_hdlc = 0x01;
    static constexpr u8 mask_cbit = 0x80;

    Address(Type, std::string, u8, u8, bool, bool);

    Type m_type;
    std::string m_name;
    u8 m_ssid;
    u8 m_reserved;
    bool m_cbit : 1;
    bool m_end_byte : 1;
};

}
