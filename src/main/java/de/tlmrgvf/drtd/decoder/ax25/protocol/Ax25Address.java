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

import de.tlmrgvf.drtd.utils.Utils;

public final class Ax25Address {
    public final static byte MASK_HDLC = 0x01;
    public final static byte MASK_CBIT = (byte) 0x80;
    public final static int ADDRESS_BLOCK_SIZE = 7;
    private final Type type;
    private final String name;
    private final byte ssid;
    private final byte reserved;
    private final boolean cBit;
    private final boolean endByte;

    public Ax25Address(Type type, String name, byte ssid, byte reserved, boolean cBit, boolean endByte) {
        this.type = type;
        this.name = name;
        this.ssid = ssid;
        this.reserved = reserved;
        this.cBit = cBit;
        this.endByte = endByte;
    }

    public static Ax25Address parseFromBlock(Type type, Byte[] bytes) {
        if (bytes.length != ADDRESS_BLOCK_SIZE)
            return null;

        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < bytes.length - 1; ++i) {
            builder.append((char) ((0xFF & bytes[i]) >> 1));
            if (Utils.compareBit(true, bytes[i], MASK_HDLC)) return null;
        }

        byte ssidByte = bytes[ADDRESS_BLOCK_SIZE - 1];
        return new Ax25Address(type,
                builder.toString(),
                (byte) ((ssidByte >> 1) & 0xF),
                (byte) ((ssidByte >> 5) & 0x3),
                Utils.compareBit(true, ssidByte, MASK_CBIT),
                Utils.compareBit(true, ssidByte, MASK_HDLC));
    }

    public boolean isEndByte() {
        return endByte;
    }

    public Type getType() {
        return type;
    }

    public String getName() {
        return name;
    }

    public boolean isRepeated() {
        return type == Type.REPEATER && cBit;
    }

    public boolean isCommand() {
        return (type == Type.SOURCE && !cBit) || (type == Type.DESTINATION && cBit);
    }

    public byte getSsid() {
        return ssid;
    }

    public byte getReserved() {
        return reserved;
    }

    public enum Type {
        DESTINATION("Destination"),
        SOURCE("Source"),
        REPEATER("Repeater");

        private final String name;

        Type(String name) {
            this.name = name;
        }

        public Object getName() {
            return name;
        }
    }
}
