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

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package de.tlmrgvf.drtd.utils.structure;


import java.util.function.IntFunction;

public class GenericRingBuffer<T> {
    private final T initialValue;
    private final IntFunction<T[]> generator;

    private T[] buffer;
    private int size;
    private int nextEmpty;
    private int count;
    private int nextRead;

    public GenericRingBuffer(IntFunction<T[]> generator, T initialValue, int size) {
        this.generator = generator;
        if (size < 1)
            throw new IllegalArgumentException("Invalid size!");

        this.initialValue = initialValue;
        resize(size);
        clear(true);
    }

    public void resize(int size) {
        if (size < 1)
            throw new IllegalArgumentException("Invalid size!");

        this.size = size;
        buffer = generator.apply(size);
        clear(true);
    }

    public int getSize() {
        return size;
    }

    public void clear(boolean initialize) {
        if (initialize) {
            for (int i = 0; i < size; ++i)
                buffer[i] = initialValue;
        }

        nextEmpty = 0;
        count = 0;
        nextRead = 0;
    }

    public T peek(int i) {
        return buffer[(i + nextEmpty) % size];
    }

    public boolean isFull() {
        return count == size;
    }

    public int getCount() {
        return count;
    }

    public int getNextEmptyPosition() {
        return nextEmpty;
    }

    public T pop() {
        if (count == 0) return null;

        T ret = buffer[nextRead];
        nextRead = (nextRead + 1) % size;
        count--;
        return ret;
    }

    public T push(T value) {
        T removed = buffer[nextEmpty];
        buffer[nextEmpty] = value;
        nextEmpty = (nextEmpty + 1) % size;
        count = Math.min(count + 1, size);
        return removed;
    }

    public T[] getContents() {
        return buffer;
    }
}
