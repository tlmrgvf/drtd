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

package de.tlmrgvf.drtd.utils.structure;

public final class BitBuffer {
    private final static int MAX_BITS = 64;
    private final static long ALL_ONES = 0xFFFFFFFFFFFFFFFFL;

    private final boolean lsbPushedLast;
    private final int bitCount;
    private final long oldMask;
    private final long resultMask;

    private long buffer;
    private int bitCounter;

    /**
     * @param lsbFirst true if the least significant bit gets pushed first
     */
    public BitBuffer(boolean lsbFirst) {
        this(lsbFirst, 8);
    }

    public BitBuffer(boolean lsbPushedLast, int bitCount) {
        if (bitCount > MAX_BITS || bitCount < 2)
            throw new IllegalArgumentException("Invalid bit count!");

        this.bitCount = bitCount;
        this.lsbPushedLast = lsbPushedLast;
        oldMask = 1 << (bitCount - 1);
        resultMask = ~(ALL_ONES << bitCount);
    }

    public boolean get(int i) {
        return ((1 << i) & buffer) != 0;
    }

    public Boolean push(Boolean push) {
        long bit = push ? 1 : 0;
        long old;

        if (lsbPushedLast) {
            bit <<= bitCount - 1;
            old = buffer & 1;
            buffer >>= 1;
            buffer = buffer & ~oldMask | bit;
        } else {
            old = buffer & oldMask;
            buffer = ((buffer << 1) & resultMask) | bit;
        }

        bitCounter = (bitCounter + 1) % bitCount;

        return old != 0;
    }

    public void resetBitCounter() {
        bitCounter = 0;
    }

    public void reset() {
        buffer = 0;
        resetBitCounter();
    }

    public int getBitCount() {
        return bitCount;
    }

    public void setBuffer(char buffer) {
        this.buffer = buffer;
    }

    public boolean bitsAligned() {
        return bitCounter == 0;
    }

    public long getBuffer() {
        return buffer & resultMask;
    }
}
