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

class RingBufferTest {

    @Test
    void clear() {
        RingBuffer buffer = new RingBuffer(10);
        for (int i = 0; i < buffer.getSize() * 3; ++i)
            buffer.push((float) i);

        buffer.clear(true);
        for (float v : buffer.getContents())
            assertEquals(0F, v);
    }

    @Test
    void peek() {
        RingBuffer buffer = new RingBuffer(42);
        for (int i = 0; i < buffer.getSize() * 4; ++i)
            buffer.push((float) i);

        for (int i = 0; i < buffer.getSize(); ++i)
            assertEquals(126 + i, buffer.peek(i));
    }

    @Test
    void isFull() {
        RingBuffer buffer = new RingBuffer(42);
        for (int i = 0; i < buffer.getSize() * 4; ++i)
            buffer.push((float) i);

        assertTrue(buffer.isFull());
        buffer.pop();

        assertFalse(buffer.isFull());
        for (int i = 0; i < buffer.getSize() * 2; ++i)
            buffer.pop();

        assertFalse(buffer.isFull());
    }

    @Test
    void getCount() {
        RingBuffer buffer = new RingBuffer(42);
        for (int i = 0; i < buffer.getSize() * 4; ++i)
            buffer.push((float) i);

        assertEquals(42, buffer.getCount());
        buffer.pop();
        assertEquals(41, buffer.getCount());

        for (int i = 0; i < buffer.getSize() * 2; ++i)
            buffer.pop();

        assertEquals(0, buffer.getCount());
    }

    @Test
    void pop() {
        RingBuffer buffer = new RingBuffer(42);
        for (int i = 0; i < buffer.getSize() * 2; ++i)
            buffer.push((float) i);

        for (int i = 0; i < buffer.getSize(); ++i)
            assertEquals(42 + i, buffer.pop());
    }

    @Test
    void push() {
        RingBuffer buffer = new RingBuffer(10);
        for (int i = 0; i < buffer.getSize() * 1.5; ++i)
            buffer.push((float) i);

        assertArrayEquals(new Float[]{10F, 11F, 12F, 13F, 14F, 5F, 6F, 7F, 8F, 9F}, buffer.getContents());

    }
}