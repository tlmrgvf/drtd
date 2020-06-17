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

public enum Pid {
    ISO_8208(0x01, "ISO 8208/CCITT X.25 PLP "),
    TCP_COMP(0x06, "Compressed TCP/IP packet (RFC 1144)"),
    TCP_UNCOMP(0x07, "Uncompressed TCP/IP packet (RFC 1144)"),
    SEGMENTATION(0x08, "Segmentation fragment"),
    TEXNET(0xC3, "TEXNET datagram protocol"),
    LQP(0xC4, "Link Quality Protocol"),
    APPLE_TALK(0xCA, "Appletalk"),
    APPLE_TALK_ARP(0xCB, "Appletalk ARP"),
    ARPA(0xCC, "ARPA Internet Protocol"),
    ARPA_ADDR(0xCD, "ARPA Address resolution"),
    FLEX_NET(0xCE, "FlexNet"),
    NET_ROM(0xCF, "NET/ROM"),
    NO_LAYER_3(0xF0, "No layer 3 protocol implemented"),
    UNKNOWN(0, "Unknown/Not yet implemented"),
    AX25_LAYER_3(0xDD, "AX.25 layer 3 implemented");

    private final byte value;
    private final String description;

    Pid(int value, String description) {
        this.value = (byte) value;
        this.description = description;
    }

    public static Pid getByByte(byte pid) {
        for (Pid t : values()) {
            if (t.value == pid) return t;
        }

        pid >>= 4;
        pid &= 3;
        return (pid == 1 || pid == 2) ? AX25_LAYER_3 : UNKNOWN;
    }

    public byte getValue() {
        return value;
    }

    public String getDescription() {
        return description;
    }
}