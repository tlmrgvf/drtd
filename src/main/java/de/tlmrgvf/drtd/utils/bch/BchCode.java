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

import de.tlmrgvf.drtd.Drtd;

import java.util.logging.Logger;

public final class BchCode {
    public final static long INVALID = 0xFFFFFFFFFFFFFFFFL;
    private final static Logger LOGGER = Drtd.getLogger(BchCode.class);
    private final EncodingType type;
    private final FiniteField field;
    private final Z2Polynomial generatorPolynomial;
    private final int n;
    private final int k;
    private final int t;

    public BchCode(EncodingType type, Z2Polynomial generatorPolynomial, int n, int k, int t) {
        if (n < k || t < 0 || k < 0 || n < 1)
            throw new IllegalArgumentException("Invalid n, k or t!");

        int order = (int) Math.ceil(Math.log10(n + 1) / Math.log10(2));

        this.type = type;
        this.field = new FiniteField(order);
        this.generatorPolynomial = generatorPolynomial;
        this.n = n;
        this.k = k;
        this.t = t;
    }

    public long encodeMessage(long message) {
        Z2Polynomial messagePoly = new Z2Polynomial(message);
        assert messagePoly.degree() < k;

        if (type == EncodingType.FACTOR) {
            return messagePoly.multiply(generatorPolynomial).getCoefficients();
        } else if (type == EncodingType.PREFIX) {
            Z2Polynomial codePrefix = new Z2Polynomial(message << (n - k)); //multiply by x^(n-k)
            return codePrefix.add(codePrefix.remainder(generatorPolynomial)).getCoefficients(); //subtract remainder
        }

        assert false;
        return INVALID;
    }

    public long correctCodeword(long codeword) {
        Z2Polynomial received = new Z2Polynomial(codeword);
        assert received.degree() < n;

        boolean errorsDetected = false;
        Z2Polynomial[] syndromes = new Z2Polynomial[2 * t];
        for (int i = 0; i < syndromes.length; ++i) {
            syndromes[i] = field.calculateSyndrome(received, i + 1);
            if (!syndromes[i].isZero())
                errorsDetected = true;
        }

        if (!errorsDetected)
            return received.getCoefficients();

        Z2Polynomial[] coefficients = calculateErrorLocatorPolynomial(syndromes);
        if (coefficients == null) {
            LOGGER.finer("Non-zero syndromes but empty polynomial? Assuming codeword is correct");
            return received.getCoefficients();
        }

        LOGGER.finer("Errors detected, attempting correction...");

        int zeroesFound = 0;
        for (long root : field.getPrimitiveRoots()) {
            Z2Polynomial rootPoly = new Z2Polynomial(root);
            Z2Polynomial sum = new Z2Polynomial(0b1);
            int power = 1;

            for (Z2Polynomial coefficient : coefficients) {
                sum = sum.add(field.multiplyRoots(coefficient, field.powerOfRoot(rootPoly, power)));
                ++power;
            }

            if (sum.isZero()) {
                ++zeroesFound;
                int location = (field.getPrimitiveRootCount() - field.getRootExponent(new Z2Polynomial(root)))
                        % field.getPrimitiveRootCount();
                codeword ^= 1 << location;
                LOGGER.finest("Corrected error at bit " + location);
            }
        }

        if (zeroesFound == 0) {
            LOGGER.finer("Too many errors, unable to correct!");
            return INVALID;
        }

        return codeword;
    }

    public long decodeMessage(long word) {
        Z2Polynomial codeword = new Z2Polynomial(word);

        if (type == EncodingType.FACTOR) {
            return codeword.divide(generatorPolynomial).getCoefficients();
        } else if (type == EncodingType.PREFIX) {
            long coefficients = codeword.getCoefficients();
            coefficients >>= 1;
            coefficients &= ~Z2Polynomial.MSB_MASK;
            coefficients >>= n - k - 1;

            return coefficients;
        }

        assert false;
        return INVALID;
    }

    private FFMatrix buildSyndromeVector(int v, Z2Polynomial[] syndromes) {
        Z2Polynomial[][] values = new Z2Polynomial[v][1];
        for (int o = 0; o < v; ++o) {
            values[o][0] = syndromes[v + o];
        }

        return new FFMatrix(field, values);
    }

    private FFMatrix buildSyndromeMatrix(Z2Polynomial[] syndromes) {
        assert syndromes.length == 2 * t;

        Z2Polynomial[][] values = new Z2Polynomial[t][t];
        for (int o = 0; o < t; ++o) {
            System.arraycopy(syndromes, o, values[o], 0, t);
        }

        return new FFMatrix(field, values);
    }

    private Z2Polynomial[] calculateErrorLocatorPolynomial(Z2Polynomial[] syndromes) {
        FFMatrix syndromeMatrix = buildSyndromeMatrix(syndromes);

        while (syndromeMatrix.getRows() > 0) {
            if (syndromeMatrix.determinant().isZero()) {
                syndromeMatrix = syndromeMatrix.shrink();
            } else {
                FFMatrix coeffs = syndromeMatrix.inverse().multiply(
                        buildSyndromeVector(syndromeMatrix.getRows(), syndromes));
                Z2Polynomial[] result = new Z2Polynomial[coeffs.getRows()];

                for (int i = 0; i < result.length; ++i)
                    result[i] = coeffs.getValues()[result.length - i - 1][0];

                return result;
            }
        }

        return null;
    }

    public enum EncodingType {
        FACTOR,
        PREFIX
    }
}
