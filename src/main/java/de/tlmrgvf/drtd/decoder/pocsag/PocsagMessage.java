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

import java.util.ArrayList;

public final class PocsagMessage {
    /* https://www.raveon.com/pdfiles/AN142(POCSAG).pdf */

    public final static int CODEWORDS_PER_BATCH = 16;
    private final static char[] NUMERIC_MAP =
            new char[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', 'U', ' ', '-', ')', '('};

    private final PocsagData address;
    private final String alphanumericalContents;
    private final String numericalContents;
    private final boolean containsInvalidCodeword;
    private final boolean containsData;

    public PocsagMessage(PocsagData address,
                         String alphanumericalContents,
                         String numericalContents,
                         boolean containsInvalidCodeword,
                         boolean containsData) {
        this.address = address;
        this.alphanumericalContents = alphanumericalContents;
        this.numericalContents = numericalContents;
        this.containsInvalidCodeword = containsInvalidCodeword;
        this.containsData = containsData;
    }

    public static MessageBuilder builder() {
        return new MessageBuilder();
    }

    public static PocsagMessage[] fromData(PocsagData[] dataArray) {
        var messages = new ArrayList<PocsagMessage>();
        var builder = PocsagMessage.builder();

        for (var data : dataArray) {
            if (data == null) {
                builder.setContainsInvalidCodeword();
                continue;
            }

            if (data.getType() == PocsagData.Type.ADDRESS) {
                if (builder.isValid())
                    messages.add(builder.build());

                builder = PocsagMessage.builder();
                builder.setAddress(data);
            } else if (data.getType() == PocsagData.Type.DATA) {
                builder.appendData(data);
            }
        }

        if (builder.isValid())
            messages.add(builder.build());

        return messages.toArray(PocsagMessage[]::new);
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

        public PocsagMessage build() {
            return new PocsagMessage(
                    address,
                    alphaBuilder.toString(),
                    numericalBuilder.toString(),
                    containsInvalidCodeword,
                    containsData
            );
        }
    }
}
