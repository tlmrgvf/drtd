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

package de.tlmrgvf.drtd.utils.bch;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

class BchCodeTest {

    final static BchCode FACTOR_CODE = new BchCode(BchCode.EncodingType.FACTOR,
            new Z2Polynomial(0b11101101001),
            new Z2Polynomial(0b100101),
            31,
            21,
            2);

    final static BchCode PREFIX_CODE = new BchCode(BchCode.EncodingType.PREFIX,
            new Z2Polynomial(0b11101101001),
            new Z2Polynomial(0b100101),
            31,
            21,
            2);

    final static long MESSAGE_ONE = 0b101101110111101111101;
    final static long MESSAGE_TWO = 0b110001010000011110101;

    @Test
    void encodeMessage() {
        assertEquals(0b1100111010010111101011101110101, FACTOR_CODE.encodeMessage(MESSAGE_ONE));
        assertEquals(0b1100010100000111101011111110100, PREFIX_CODE.encodeMessage(MESSAGE_TWO));
    }

    @Test
    void correctCodeword() {
        long message = FACTOR_CODE.encodeMessage(0b110101010000101011101);
        assertEquals(0b110101010000101011101, FACTOR_CODE.decodeMessage(FACTOR_CODE.correctCodeword(message)));
        message ^= 0b1000000;
        assertEquals(0b110101010000101011101, FACTOR_CODE.decodeMessage(FACTOR_CODE.correctCodeword(message)));
        message ^= 0b1000;
        assertEquals(0b110101010000101011101, FACTOR_CODE.decodeMessage(FACTOR_CODE.correctCodeword(message)));

        message = PREFIX_CODE.encodeMessage(0b001010101111101100011);
        assertEquals(0b001010101111101100011, PREFIX_CODE.decodeMessage(PREFIX_CODE.correctCodeword(message)));
        message ^= 0b1;
        assertEquals(0b001010101111101100011, PREFIX_CODE.decodeMessage(PREFIX_CODE.correctCodeword(message)));
        message ^= 0b1000000000;
        assertEquals(0b001010101111101100011, PREFIX_CODE.decodeMessage(PREFIX_CODE.correctCodeword(message)));
    }

    @Test
    void decodeMessage() {
        assertEquals(MESSAGE_ONE, FACTOR_CODE.decodeMessage(0b1100111010010111101011101110101));
        assertEquals(MESSAGE_TWO, PREFIX_CODE.decodeMessage(0b1100010100000111101011111110100));
    }
}