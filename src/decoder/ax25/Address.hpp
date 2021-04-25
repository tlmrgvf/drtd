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
