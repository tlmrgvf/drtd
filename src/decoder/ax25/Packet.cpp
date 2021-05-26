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
#include "Address.hpp"
#include "Packet.hpp"
#include <chrono>
#include <cstring>
#include <ctime>
#include <util/Logger.hpp>
#include <util/Util.hpp>

static Logger s_logger("AX25 Packet");

using namespace Dsp::AX25Protocol;

Packet::Packet(Type type,
               Address source,
               Address destination,
               std::vector<Address> repeaters,
               u8 control,
               bool poll,
               Buffer<u8> data,
               PacketContents contents)
    : m_type(type)
    , m_source(source)
    , m_destination(destination)
    , m_repeaters(std::move(repeaters))
    , m_control(control)
    , m_poll(poll)
    , m_data(std::move(data))
    , m_contents(contents) {
}

const char* Packet::pid_by_byte(u8 byte) {
    switch (byte) {
    case 0x01:
        return "ISO 8208/CCITT X.25 PLP ";
    case 0x06:
        return "Compressed TCP/IP packet (RFC 1144)";
    case 0x07:
        return "Uncompressed TCP/IP packet (RFC 1144)";
    case 0x08:
        return "Segmentation fragment";
    case 0xC3:
        return "TEXNET datagram protocol";
    case 0xC4:
        return "Link Quality Protocol";
    case 0xCA:
        return "Appletalk";
    case 0xCB:
        return "Appletalk ARP";
    case 0xCC:
        return "ARPA Internet Protocol";
    case 0xCD:
        return "ARPA Address resolution";
    case 0xCE:
        return "FlexNet";
    case 0xCF:
        return "NET/ROM";
    case 0xF0:
        return "No layer 3 protocol implemented";
    case 0xDD:
        return "AX.25 layer 3 implemented";
    }

    return "Unknown/Not yet implemented";
}

const char* Packet::receive_type_by_byte(u8 receive_type) {

    switch (receive_type) {
    case 0:
        return "Receive ready";
    case 1:
        return "Receive not ready";
    case 2:
        return "Reject";
    }

    return "Unknown";
}

const char* Packet::control_type_by_byte(u8 byte) {
    switch (byte) {
    case 0x0F:
        return "Set asynchronous balanced mode extended";
    case 0x07:
        return "Set asynchronous balanced mode";
    case 0x08:
        return "Disconnect";
    case 0x03:
        return "Disconnected mode";
    case 0x0C:
        return "Unnumbered acknowledge";
    case 0x11:
        return "Frame reject";
    case 0:
        return "Unnumbered information";
    case 0x1C:
        return "Test";
    case 0x17:
        return "Exchange identifications";
    }

    return "Unknown control type";
}

bool Packet::uses_information(u8 byte) {
    return byte == 0x11 || byte == 0 || byte == 0x1C || byte == 0x17 || byte == 0xFF;
}

std::optional<Packet> Packet::parse(const std::vector<u8>& bytes) {
    static Logger logger("AX25 Packet");

    if (bytes.size() < min_packet_size) {
        logger.warning() << "Packet too short";
        return {};
    }

    unsigned offset = 1; //Skip 0
    std::vector<Address> repeaters;

    auto address = Address::parse(Address::Type::Destination, bytes, offset);
    if (!address.has_value()) {
        logger.warning() << "Could not parse destination address";
        return {};
    }

    offset += Address::address_block_size;
    Address destination = address.value();

    address = Address::parse(Address::Type::Source, bytes, offset);
    if (!address.has_value()) {
        logger.warning() << "Could not parse source address";
        return {};
    }

    offset += Address::address_block_size;
    Address source = address.value();

    if (!source.is_end_byte()) {
        for (unsigned i = 0; i < max_repeaters; ++i) {
            address = Address::parse(Address::Type::Repeater, bytes, offset);
            if (!address.has_value()) {
                logger.warning() << "Invalid address block";
                return {};
            }

            offset += Address::address_block_size;
            repeaters.push_back(address.value());
            if (address->is_end_byte())
                break;
        }
    }

    u8 control = bytes[offset++];
    Type type = (control % 2) ? (control & 2) ? Type::Unnumbered : Type::Supervisory : Type::Information;

    bool poll = control & poll_mask;
    //FIXME: The FCS byte is currently just appended to the data, we should check it
    //NOTE: APRS seems to not use the FCS, is there a way to detect if it is included?
    //FIXME: mod 128 support not implemented

    if (type == Type::Information) {
        u8 pid = bytes[offset++];
        if (pid == pid_escape)
            pid = bytes[offset++];

        Buffer<u8> data(bytes.size() - offset);
        std::memcpy(data.ptr(), bytes.data() + offset, data.size());

        InformationData information;
        information.pid = pid_by_byte(pid);
        information.receive_sequence_number = control >> 1 & 0x07;
        information.send_sequence_number = control >> 5 & 0x07;

        return Packet(type, source, destination, std::move(repeaters), control, poll, std::move(data), PacketContents { .information = information });
    } else if (type == Type::Supervisory) {
        SupervisoryData supervisory;
        supervisory.receive_type = receive_type_by_byte(control >> 2 & 0x3);
        supervisory.receive_sequence_number = control << 5 & 0x7;

        return Packet(type, source, destination, std::move(repeaters), control, poll, {}, PacketContents { .supervisory = supervisory });
    } else if (type == Type::Unnumbered) {
        UnnumberedData unnumbered;
        u8 control_type = control >> 2;
        control_type = (control_type & 0x34) >> 1 | (control_type & 0x3);
        if (!control_type) { // Unnumbered information
            u8 pid = bytes[offset++];
            if (pid == pid_escape)
                pid = bytes[offset++];
            unnumbered.pid = pid_by_byte(pid);
        } else {
            unnumbered.pid = "Packet has no PID";
        }
        unnumbered.control_type = control_type_by_byte(control_type);

        Buffer<u8> data;
        if (uses_information(control_type)) {
            data = Buffer<u8>(bytes.size() - offset);
            std::memcpy(data.ptr(), bytes.data() + offset, data.size());
        }

        return Packet(type, source, destination, std::move(repeaters), control, poll, data, PacketContents { .unnumbered = unnumbered });
    }

    assert(false);
    return {};
}

std::string Packet::format() const {
    std::ostringstream builder;
    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    builder << "Received at " << std::ctime(&time);
    builder << "Type: ";
    switch (m_type) {
    case Type::Information:
        builder << "Information\n";
        builder << "Pid: " << m_contents.information.pid
                << ", SSN: " << std::hex << "0x" << static_cast<unsigned>(m_contents.information.send_sequence_number)
                << ", RSN: " << static_cast<unsigned>(m_contents.information.receive_sequence_number);
        break;
    case Type::Supervisory:
        builder << "Supervisory\n";
        builder << "Receive type: " << m_contents.supervisory.receive_type
                << ", RSN: " << std::hex << "0x" << static_cast<unsigned>(m_contents.supervisory.receive_sequence_number);
        break;
    case Type::Unnumbered:
        builder << "Unnumbered\n";
        builder << m_contents.unnumbered.control_type
                << ", Pid: " << m_contents.unnumbered.pid;
        break;
    default:
        assert(false);
    }

    builder << " (0x" << std::hex << static_cast<unsigned>(m_control) << ")";

    if (is_poll())
        builder << " [Poll]";
    builder << '\n'
            << m_source.format() << "->";

    for (auto& repeater : m_repeaters)
        builder << '\n'
                << repeater.format() << "->";

    builder << '\n'
            << m_destination.format() << '\n';

    builder << ">>>\n";
    if (m_data.size()) {
        for (u8 byte : m_data)
            builder << Util::escape_ascii(byte);
    } else {
        builder << "[Packet has no data field]";
    }
    builder << "\n<<<\n\n";
    return builder.str();
}
