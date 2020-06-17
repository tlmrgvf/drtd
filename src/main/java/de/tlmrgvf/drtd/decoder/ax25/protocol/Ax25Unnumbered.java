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

import java.util.HashMap;
import java.util.Map;

public final class Ax25Unnumbered extends Ax25Packet {
    private final ControlType controlType;
    private Pid pid;

    public Ax25Unnumbered(Type type,
                          Ax25Address sourceAddress,
                          Ax25Address destinationAddress,
                          Ax25Address[] repeaters,
                          Byte[] raw,
                          byte control,
                          boolean poll) {
        super(type, sourceAddress, destinationAddress, repeaters, raw, control, poll);
        this.controlType = ControlType.getByControlByte(control);
    }

    @Override
    public Map<String, String> getPropertyMap() {
        final HashMap<String, String> map = new HashMap<>();
        map.put("Control type", controlType.getDescription());
        map.put("PID", pid.getDescription());
        return map;
    }

    public void setPid(Pid pid) {
        this.pid = pid;
    }

    public ControlType getControlType() {
        return controlType;
    }

    public enum ControlType {
        SABME(0xF, "Set asynchronous balanced mode extended", false),
        SABM(0x7, "Set asynchronous balanced mode", false),
        DISC(0x8, "Disconnect", false),
        DM(0x3, "Disconnected mode", false),
        UA(0xC, "Unnumbered acknowledge", false),
        FRMR(0x11, "Frame reject", true),
        UI(0, "Unnumbered information", true),
        TEST(0x1C, "Test", true),
        XID(0x17, "Exchange identifications", true),
        UNKNOWN(0xFF, "Unknown control type", true);

        private final byte value;
        private final String description;
        private final boolean usesInformation;

        ControlType(int value, String description, boolean usesInformation) {
            this.value = (byte) value;
            this.description = description;
            this.usesInformation = usesInformation;
        }

        public static ControlType getByControlByte(byte control) {
            control >>= 2;
            control = (byte) (((control & 0x34) >> 1) | (control & 0x3));

            for (ControlType t : values()) {
                if (t.value == control) return t;
            }
            Drtd.getLogger(ControlType.class).warning("Unknown control type 0x" + Integer.toHexString((char) control));
            return UNKNOWN;
        }

        public boolean usesInformation() {
            return usesInformation;
        }

        public byte getValue() {
            return value;
        }

        public String getDescription() {
            return description;
        }
    }
}
