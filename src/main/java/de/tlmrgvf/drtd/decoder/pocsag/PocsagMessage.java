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

package de.tlmrgvf.drtd.decoder.pocsag;

import de.tlmrgvf.drtd.utils.Utils;

public final class PocsagMessage {
    /* https://www.raveon.com/pdfiles/AN142(POCSAG).pdf */

    public final static int CODEWORDS_PER_BATCH = 16;
    private final static char[] NUMERIC_MAP =
            new char[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', 'U', ' ', '-', ')', '('};

    private final ContentType contentType;
    private final PocsagData address;
    private final String alphanumericalContents;
    private final String numericalContents;
    private final boolean containsInvalidCodeword;
    private final boolean containsData;
    private final int baudRate;

    public PocsagMessage(ContentType contentType,
                         PocsagData address,
                         String alphanumericalContents,
                         String numericalContents,
                         boolean containsInvalidCodeword,
                         boolean containsData,
                         int baudRate) {
        this.contentType = contentType;
        this.address = address;
        this.alphanumericalContents = alphanumericalContents;
        this.numericalContents = numericalContents;
        this.containsInvalidCodeword = containsInvalidCodeword;
        this.containsData = containsData;
        this.baudRate = baudRate;
    }

    public boolean containsInvalidCodeword() {
        return containsInvalidCodeword;
    }

    public boolean containsData() {
        return containsData;
    }

    public String getAlphanumericalContents() {
        return alphanumericalContents;
    }

    public PocsagData getAddress() {
        return address;
    }

    public String getNumericalContents() {
        return numericalContents;
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("POCSAG");
        builder.append(baudRate);

        builder.append(" ; Address: ");
        builder.append(address == null ? "-" : address.toString());

        builder.append(" ; Function: ");
        builder.append(address == null ? "-" : address.getFunctionBits());

        if (containsInvalidCodeword)
            builder.append(" ; Errors detected!");

        if (containsData) {
            if (contentType == ContentType.ALPHANUMERIC || contentType == ContentType.BOTH) {
                builder.append("\n\tAlphanumeric: ");
                builder.append(alphanumericalContents);
            }

            if (contentType == ContentType.NUMERIC || contentType == ContentType.BOTH) {
                builder.append("\n\tNumeric: ");
                builder.append(numericalContents);
            }
        }

        builder.append('\n');
        return builder.toString();
    }

    public enum ContentType {
        NONE,
        NUMERIC,
        ALPHANUMERIC,
        BOTH;

        public static ContentType fromName(String name) {
            for (ContentType t : values())
                if (t.name().equalsIgnoreCase(name))
                    return t;

            return null;
        }

        @Override
        public String toString() {
            return name().charAt(0) + name().substring(1).toLowerCase();
        }
    }

    public static class MessageBuilder {
        private final StringBuilder alphaBuilder;
        private final StringBuilder numericalBuilder;

        private PocsagData address;
        private boolean containsInvalidCodeword;
        private char rawDataBits = 0;
        private int asciiBits = 0;
        private boolean valid = false;
        private boolean containsData = false;

        public MessageBuilder() {
            this.alphaBuilder = new StringBuilder();
            this.numericalBuilder = new StringBuilder();
        }

        public void setContainsInvalidCodeword() {
            containsInvalidCodeword = true;
        }

        public boolean isValid() {
            return valid;
        }

        public void setAddress(PocsagData address) {
            this.address = address;
            valid = true;
        }

        public void appendData(PocsagData data) {
            if (data.getType() == PocsagData.Type.ADDRESS) {
                setAddress(data);
                return;
            } else if (data.getType() == PocsagData.Type.IDLE) {
                return;
            }

            containsData = true;
            valid = true;
            int contents = data.getContents();

            for (int i = 0; i < PocsagData.DATA_BITS && contents != 0; ++i) {
                rawDataBits >>= 1;

                if ((contents & PocsagData.DATA_MSB_MASK) != 0)
                    rawDataBits |= 0x40;

                if (asciiBits == 6) {
                    alphaBuilder.append(Utils.escapeAscii(rawDataBits));
                    asciiBits = 0;
                    rawDataBits = 0;
                } else {
                    ++asciiBits;
                }

                contents <<= 1;
            }

            contents = data.getContents();
            for (int i = 0; i < PocsagData.DATA_BITS / 4; ++i)
                numericalBuilder.append(NUMERIC_MAP[((contents & 0xF0000) >> 16)]);
        }

        public PocsagMessage build(ContentType type, int baudRate) {
            return new PocsagMessage(
                    type,
                    address,
                    alphaBuilder.toString(),
                    numericalBuilder.toString(),
                    containsInvalidCodeword,
                    containsData,
                    baudRate
            );
        }
    }
}
