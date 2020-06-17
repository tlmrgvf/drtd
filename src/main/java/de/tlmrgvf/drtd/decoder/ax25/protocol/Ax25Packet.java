/*
 *
 * BSD 2-Clause License
 *
 * Copyright (c) 2020, Till Mayer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package de.tlmrgvf.drtd.decoder.ax25.protocol;

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.utils.Utils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;

public abstract class Ax25Packet {
    public final static char MAGIC_FLAG = 0x7E;
    public final static byte POLL_MASK = 0x10;
    public final static int MAX_REPEATERS = 8;
    public final static int MIN_SIZE = 15; //120 bits
    private final static Logger LOGGER = Drtd.getLogger(Ax25Packet.class);

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

    private final Ax25Address sourceAddress, destinationAddress;
    private final Type type;
    private final Ax25Address[] repeaters;
    private final byte control;
    private final boolean poll;
    private final Byte[] raw;
    private Byte[] data;

    public Ax25Packet(Type type,
                      Ax25Address sourceAddress,
                      Ax25Address destinationAddress,
                      Ax25Address[] repeaters,
                      Byte[] raw,
                      byte control,
                      boolean poll) {
        this.sourceAddress = sourceAddress;
        this.destinationAddress = destinationAddress;
        this.repeaters = repeaters;
        this.type = type;
        this.control = control;
        this.raw = raw;
        this.poll = poll;
    }

    public static Ax25Packet parse(final Byte[] raw) {
        if (raw.length < MIN_SIZE) {
            logParseError("Packet too short");
            return null;
        }

        int offset = 0;
        List<Ax25Address> repeaters = new ArrayList<>(MAX_REPEATERS);

        Ax25Address destAddr = readAddress(Ax25Address.Type.DESTINATION, raw, 0);
        Ax25Address sourceAddr = readAddress(Ax25Address.Type.SOURCE, raw, 1);

        offset += 2 * Ax25Address.ADDRESS_BLOCK_SIZE;

        if (sourceAddr == null || destAddr == null) {
            logParseError("Source/Destination address is not valid");
            return null;
        }

        if (!sourceAddr.isEndByte()) {
            Ax25Address addr = sourceAddr;

            for (int i = 0; !addr.isEndByte() && i < MAX_REPEATERS; ++i) {
                addr = readAddress(Ax25Address.Type.REPEATER, raw, 2 + i);
                if (addr == null) {
                    logParseError("Invalid address block");
                    return null;
                }
                offset += Ax25Address.ADDRESS_BLOCK_SIZE;
                repeaters.add(addr);
            }
        }

        byte contol = raw[offset++];
        Type type = Type.getByControl(contol);
        Ax25Address[] repeaterArray = repeaters.toArray(Ax25Address[]::new);
        boolean poll = Utils.compareBit(true, contol, POLL_MASK);
        //FIXME: The FCS byte is currently just appended to the data, we should check it
        //NOTE: APRS seems to not use the FCS, is there a way to detect if it is included?
        //FIXME: mod 128 support not implemented

        if (type == Type.INFORMATION) {
            byte pid = raw[offset++];
            if (pid == Ax25Information.PID_ESCAPE) pid = raw[offset++];

            Byte[] bytes = Arrays.copyOfRange(raw, offset, raw.length);
            Ax25Information pack = new Ax25Information(
                    type,
                    sourceAddr,
                    destAddr,
                    repeaterArray,
                    raw,
                    contol,
                    poll,
                    pid
            );
            pack.setData(bytes);
            return pack;
        } else if (type == Type.SUPERVISORY) {
            return new Ax25Supervisory(type, sourceAddr, destAddr, repeaterArray, raw, contol, poll);
        } else if (type == Type.UNNUMBERED) {
            Ax25Unnumbered frame = new Ax25Unnumbered(type, sourceAddr, destAddr, repeaterArray, raw, contol, poll);
            if (frame.getControlType() == null) {
                logParseError("Unknown control type");
                return null;
            }

            if (frame.getControlType() == Ax25Unnumbered.ControlType.UI) {
                byte pid = raw[offset++];
                if (pid == Ax25Information.PID_ESCAPE) pid = raw[offset++];
                frame.setPid(Pid.getByByte(pid));
            }

            if (frame.getControlType().usesInformation()) {
                Byte[] bytes = Arrays.copyOfRange(raw, offset, raw.length);
                frame.setData(bytes);
            }
            return frame;
        }

        logParseError("Invalid type");
        return null;
    }

    private static Ax25Address readAddress(Ax25Address.Type type, Byte[] raw, int offset) {
        offset *= Ax25Address.ADDRESS_BLOCK_SIZE;
        if (raw.length <= offset + 7) {
            logParseError("Address block not complete");
            return null;
        }

        Byte[] buf = new Byte[Ax25Address.ADDRESS_BLOCK_SIZE];
        System.arraycopy(raw, offset, buf, 0, Ax25Address.ADDRESS_BLOCK_SIZE);

        return Ax25Address.parseFromBlock(type, buf);
    }

    private static void logParseError(String description) {
        LOGGER.info("Could not parse packet: " + description);
    }

    public Byte[] getRaw() {
        return raw;
    }

    public abstract Map<String, String> getPropertyMap();

    public Byte[] getData() {
        return data;
    }

    public void setData(Byte[] data) {
        this.data = data;
    }

    public boolean isPoll() {
        return poll;
    }

    public Ax25Address getSourceAddress() {
        return sourceAddress;
    }

    public Ax25Address getDestinationAddress() {
        return destinationAddress;
    }

    public Ax25Address[] getRepeaters() {
        return repeaters;
    }

    public Type getType() {
        return type;
    }

    public byte getControl() {
        return control;
    }

    public enum Type {
        INFORMATION("Information"),
        SUPERVISORY("Supervisory"),
        UNNUMBERED("Unnumbered");

        private final String name;

        Type(String name) {
            this.name = name;
        }

        public static Type getByControl(byte control) {
            if (control % 2 == 0) {
                return INFORMATION;
            }

            return (control & 2) == 0 ? SUPERVISORY : UNNUMBERED;
        }

        public String getName() {
            return name;
        }
    }
}
