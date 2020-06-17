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

package de.tlmrgvf.drtd.utils;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

class ComplexTest {

    @Test
    void angle() {
        assertEquals((float) Math.PI / 4, new Complex(1, 1).angle());
        assertEquals((float) -Math.PI / 4, new Complex(1, -1).angle());
    }

    @Test
    void magnitude() {
        assertEquals((float) Math.sqrt(2), new Complex(1, 1).magnitude());
        assertEquals((float) Math.sqrt(2), new Complex(-1, -1).magnitude());
    }

    @Test
    void magnitudeSquared() {
        assertEquals(2F, new Complex(1, 1).magnitudeSquared());
        assertEquals(26F, new Complex(-5, -1).magnitudeSquared());
    }

    @Test
    void add() {
        assertEquals(new Complex(0, 0), new Complex(-1, 5).add(new Complex(1, -5)));
    }

    @Test
    void subtract() {
        assertEquals(new Complex(8, -5),
                new Complex(10, 0).subtract(new Complex(2, 5)));
    }

    @Test
    void multiply() {
        assertEquals(new Complex(-40, 40),
                new Complex(4, 8).multiply(new Complex(2, 6)));
    }

    @Test
    void normalize() {
        assertEquals(1F, new Complex(12, 34).normalize().magnitude());
    }

    @Test
    void scale() {
        assertEquals(new Complex(4, 6), new Complex(2, 3).scale(2));
    }
}