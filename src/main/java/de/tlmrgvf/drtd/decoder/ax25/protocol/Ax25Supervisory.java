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

import java.util.HashMap;
import java.util.Map;

public final class Ax25Supervisory extends Ax25Packet {

    private final ReceiveType receiveType;
    private final byte receiveSequenceNumber;

    public Ax25Supervisory(Type type,
                           Ax25Address sourceAddress,
                           Ax25Address destinationAddress,
                           Ax25Address[] repeaters,
                           Byte[] raw,
                           byte control,
                           boolean poll) {
        super(type, sourceAddress, destinationAddress, repeaters, raw, control, poll);
        this.receiveType = ReceiveType.getByControlByte(control);
        this.receiveSequenceNumber = (byte) (0x7 & control << 5);
    }

    @Override
    public Map<String, String> getPropertyMap() {
        final HashMap<String, String> map = new HashMap<>();
        map.put("Receive type", receiveType.getDescription());
        map.put("Receive sequence number", String.format("0x%02x", receiveSequenceNumber));
        return map;
    }

    public enum ReceiveType {
        RECEIVE_READY(0, "Receive ready"),
        RECEIVE_NOT_READY(1, "Receive not ready"),
        REJECT(2, "Reject");

        private final byte value;
        private final String description;

        ReceiveType(int value, String description) {
            this.value = (byte) value;
            this.description = description;
        }

        public static ReceiveType getByControlByte(byte control) {
            control >>= 2;
            control &= 0x3;

            for (ReceiveType t : values()) {
                if (t.value == control) return t;
            }
            return null;
        }

        public byte getValue() {
            return value;
        }

        public String getDescription() {
            return description;
        }
    }
}
