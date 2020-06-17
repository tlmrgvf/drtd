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

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class BitBufferTest {

    @Test
    void get() {
        BitBuffer buffer = new BitBuffer(true, 12);
        for (int i = 0; i < buffer.getBitCount(); i++)
            buffer.push(i % 2 == 0);

        for (int i = 0; i < buffer.getBitCount(); i++)
            assertEquals(i % 2 == 0, buffer.get(i));

        buffer = new BitBuffer(false, 32);
        for (int i = 0; i < buffer.getBitCount(); i++)
            buffer.push(i % 2 == 0);

        for (int i = 0; i < buffer.getBitCount(); i++)
            assertEquals(i % 2 == 0, buffer.get(buffer.getBitCount() - i - 1));
    }

    @Test
    void push() {
        BitBuffer buffer = new BitBuffer(true, 8);
        for (int i = 0; i < buffer.getBitCount(); i++)
            buffer.push(i % 2 == 0);

        assertEquals(0b1010101, buffer.getBuffer());

        buffer = new BitBuffer(false, 10);
        for (int i = 0; i < buffer.getBitCount(); i++)
            buffer.push(i % 2 == 0);

        assertEquals(0b1010101010, buffer.getBuffer());
    }

    @Test
    void bitsAligned() {
        BitBuffer buffer = new BitBuffer(true, 12);
        for (int i = 0; i < buffer.getBitCount() - 1; i++)
            buffer.push(false);

        assertFalse(buffer.bitsAligned());
        buffer.push(true);
        assertTrue(buffer.bitsAligned());
        buffer.push(false);
        assertFalse(buffer.bitsAligned());

        for (int i = 0; i < buffer.getBitCount() - 2; i++)
            buffer.push(true);

        assertFalse(buffer.bitsAligned());
        buffer.push(true);
        assertTrue(buffer.bitsAligned());
        buffer.reset();
        assertTrue(buffer.bitsAligned());
        buffer.resetBitCounter();
        assertTrue(buffer.bitsAligned());
    }
}