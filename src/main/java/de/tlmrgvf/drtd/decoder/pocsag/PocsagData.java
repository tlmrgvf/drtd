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

public final class PocsagData {
    public final static int SYNC_WORD = 0x7CD215D8;
    public final static int IDLE_WORD = 0x7A89C197;
    public final static int DATA_MSB_MASK = 0x80000;
    public final static int DATA_BITS = 20;
    public final static int CODEWORD_BITS = 32;
    public final static int PREAMBLE_COUNT = 576;
    private final Type type;
    private final byte functionBits;
    private final int contents;

    private PocsagData(Type type, byte functionBits, int contents) {
        this.type = type;
        this.functionBits = functionBits;
        this.contents = contents;
    }

    public static PocsagData fromBits(int positionInBatch, int codeword) {
        if (codeword == IDLE_WORD >> 1) //Remove parity
            return new PocsagData(Type.IDLE, (byte) -1, 0);

        codeword >>= 10; //Remove BCH code bits
        if ((codeword & 0x100000) == 0)
            return new PocsagData(Type.ADDRESS,
                    (byte) (codeword & 3),
                    ((codeword & ~3) << 1) | (positionInBatch / 2));

        return new PocsagData(Type.DATA, (byte) -1, codeword & ~0x100000);
    }

    public Type getType() {
        return type;
    }

    public byte getFunctionBits() {
        return functionBits;
    }

    public int getContents() {
        return contents;
    }

    @Override
    public String toString() {
        return Integer.toString(contents);
    }

    public enum Type {
        ADDRESS,
        DATA,
        IDLE
    }
}
