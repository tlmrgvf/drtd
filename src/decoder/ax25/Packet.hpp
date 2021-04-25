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

#include "Address.hpp"
#include <optional>
#include <string>
#include <util/Buffer.hpp>
#include <util/Types.hpp>
#include <vector>

namespace Dsp::AX25Protocol {

class Packet {
public:
    /*
         * https://www.tapr.org/pub_ax25.html
         * https://www.tapr.org/pdf/AX25.2.2.pdf
         *
         *
         * +----------+----------------+---------+--------+----------+---------+----------+
         * | Flag     | Address        | Control | (PID)  | (Data)   |  FCS    | Flag     |
         * +----------+----------------+---------+--------+----------+---------+----------+
         * | 01111110 | 112 - 560 bits | 8 bits  | 8 bits | n*8 bits | 16 bits | 01111110 |
         * +----------+----------------+---------+--------+----------+---------+----------+
         *                    |          |         |
         *                    |          |         |  +------+------------------+
         *                    |          |         |  | PID  | Layer 3 protocol |
         *                    |          |         |  +------+------------------+-------------------------------------------------------+
         *                    |          |         \- | 0x01 | ISO 8208/CCITT X.25 PLP                                                  |
         *                    |          |            | 0x06 | Compressed TCP/IP packet. Van Jacobson (RFC 1144)                        |
         *                    |          |            | 0x07 | Uncompressed TCP/IP packet. Van Jacobson (RFC 1144)                      |
         *                    |          |            | 0x08 | Segmentation fragment                                                    |
         *                    |          |            | **** | AX.25 layer 3 implemented (xx01xxxx)                                     |
         *                    |          |            | **** | AX.25 layer 3 implemented (xx10xxxx)                                     |
         *                    |          |            | 0xC3 | TEXNET datagram protocol                                                 |
         *                    |          |            | 0xC4 | Link Quality Protocol                                                    |
         *                    |          |            | 0xCA | Appletalk                                                                |
         *                    |          |            | 0xCB | Appletalk ARP                                                            |
         *                    |          |            | 0xCC | ARPA Internet Protocol                                                   |
         *                    |          |            | 0xCD | ARPA Address resolution                                                  |
         *                    |          |            | 0xCE | FlexNet                                                                  |
         *                    |          |            | 0xCF | NET/ROM                                                                  |
         *                    |          |            | 0xF0 | No layer 3 protocol implemented                                          |
         *                    |          |            | 0xFF | Escape character. Next octet contains more Level 3 protocol information. |
         *                    |          |            +------+--------------------------------------------------------------------------+
         *                    |          |
         *                    |          |       ==========================================================================================
         *                    |          |
         *                    |          |         /-
         *                    |          \--------|
         *                    |                   |  +---------------+--------------------+
         *                    |                   |  | Frame type    | Bit number (7 MSB) |
         *                    |                   |  |               +---+---+---+---+---++--+---+---+
         *                    |                   |  |               | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
         *                    |                   |  +---------------+---+---+---+---+---+---+---+---+
         *                    |                   |  | I(nformation) |    RSN    | P |    SSN    | 0 |
         *                    |                   |  +---------------+---+---+---+---+---+---+---+---+
         *                    |                   |  | S(upervisory) |    RSN    | G |   S   | 0 | 1 |
         *                    |                   |  +---------------+---+---+---+---+---+---+---+---+
         *                    |                   |  | U(nnumbered)  | M | M | M | G | M | M | 1 | 1 |
         *                    |                   |  +---------------+---+---+---+---+---+---+---+---+
         *                    |                   |
         *                    |                   |  P: Poll bit (command)
         *                    |                   |  F: Final bit (command response)
         *                    |                   |  G: Either poll or final
         *                    |                   |  RSN: Receive sequence number
         *                    |                   |  SSN: Send sequence number
         *                    |                   |
         *                    |                   |  S: +-------------------+----+
         *                    |                   |     | Receive ready     | 00 |
         *                    |                   |     | Receive not ready | 01 |
         *                    |                   |     | Reject            | 10 |
         *                    |                   |     +-------------------+----+
         *                    |                   |
         *                    |                   |  M: +--------------------------------+--------+
         *                    |                   |     | Set asynchronous balanced mode | 001P11 |
         *                    |                   |     | Disconnect                     | 010P00 |
         *                    |                   |     | Disconnected mode              | 000F11 |
         *                    |                   |     | Unnumbered acknowledge         | 011F00 |
         *                    |                   |     | Frame reject                   | 100F01 |
         *                    |                   |     | Unnumbered information         | 000G00 |
         *                    |                   |     +--------------------------------+--------+
         *                    |                   |
         *                    |                    \-
         *                    |
         *                    |           ==================================================================================================
         *                    |
         *                    |          /-
         *                    \---------|
         *                              |  +-----------------------------------------+-----------------------------------------+----------...
         *                              |  | Destination address                     | Source address                          | Repeaters...
         *                              |  +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+----------...
         *                              |  | A1  | A2  | A3  | A4  | A5  | A6  | A7  | A8  | A9  | A10 | A11 | A12 | A13 | A14 |...
         *                              |  +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+----------...
         *                              |
         *                              |  Up to a total of 10 address fields, of which 8 are special optional repeater addresses
         *                              |
         *                              |  +-----------------+--------------------+
         *                              |  | Address byte    | Bit number (7 MSB) |
         *                              |  |                 +---+---+---+---+---++--+---+---+
         *                              |  |        An       | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
         *                              |  +-----------------+---+---+---+---+---+---+---+---+
         *                              |  | A7 & A14 (SSID) | C | R | R |     SSID      | H |
         *                              |  +-----------------+---+---+---+---+---+---+---+---+
         *                              |  | Other (Data)    |             D             | H |
         *                              |  +-----------------+---+---+---+---+---+---+---+---+
         *                              |
         *                              |
         *                              |  C: A7/A14:
         *                              |                +----------------+-----------------+
         *                              |                | Dest. SSID bit | Source SSID bit |
         *                              |     +----------+----------------+-----------------+
         *                              |     | Command  |        1       |        0        |
         *                              |     | Response |        0       |        1        |
         *                              |     +----------+----------------+-----------------+
         *                              |
         *                              |    other ssids:
         *                                    has-been-repeated bit, set when sent through repeater
         *                              |
         *                              |  R: Reserved bit, should be 1
         *                              |  H: HDLC extension bit. When 0, next byte is another address bytes, when 1 end
         *                              |     of address field
         *                              |  D: Data. For call signs, ASCII left-shifted by one
         *                              |
         *                              \-
         */

    static constexpr u8 magic_flag { 0x7E };
    static constexpr u8 poll_mask { 0x10 };
    static constexpr u8 max_repeaters { 8 };
    static constexpr u8 min_packet_size { 15 }; //120 bits
    static constexpr u8 pid_escape { 0xFF };    //120 bits

    struct InformationData {
        const char* pid;
        u8 send_sequence_number;
        u8 receive_sequence_number;
    };

    struct SupervisoryData {
        const char* receive_type;
        u8 receive_sequence_number;
    };

    struct UnnumberedData {
        const char* control_type;
        const char* pid;
    };

    typedef union {
        InformationData information;
        SupervisoryData supervisory;
        UnnumberedData unnumbered;
    } PacketContents;

    static std::optional<Packet> parse(const std::vector<u8>& bytes);

    std::string format() const;
    bool is_poll() const { return m_poll; }

private:
    enum class Type : u8 {
        Information,
        Supervisory,
        Unnumbered
    };

    Packet(Type, Address, Address, std::vector<Address>, u8, bool, Buffer<u8>, PacketContents);

    static const char* pid_by_byte(u8);
    static const char* control_type_by_byte(u8);
    static const char* receive_type_by_byte(u8);
    static bool uses_information(u8);

    Type m_type;
    Address m_source;
    Address m_destination;
    std::vector<Address> m_repeaters;
    u8 m_control;
    bool m_poll;
    Buffer<u8> m_data;
    PacketContents m_contents;
};

}
