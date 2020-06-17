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

public final class FiniteField {
    private final long[] primitiveRoots;
    private final int[] primitiveRootsLog;
    private final int elements;
    private final int exponent;

    public FiniteField(int exponent, Z2Polynomial irreducible) {
        this.exponent = exponent;
        //Or else we won't be able to use an array to map roots to their exponents
        assert exponent <= Integer.SIZE - 1;
        assert exponent > 0;
        assert irreducible.degree() == exponent;

        primitiveRoots = new long[(int) (Math.pow(2, exponent) - 1)];
        elements = primitiveRoots.length;
        primitiveRootsLog = new int[primitiveRoots.length + 1];

        long alpha = 1; //Use x as the primitive root of unity
        primitiveRoots[0] = alpha; //a^0
        primitiveRootsLog[0] = -1; //There is no exponent to make a^n = 0!

        for (int i = 1; i < primitiveRoots.length; ++i) {
            alpha <<= 1; //times x
            primitiveRoots[i] = new Z2Polynomial(alpha).remainder(irreducible).getCoefficients();
            primitiveRootsLog[(int) primitiveRoots[i]] = i;
        }
    }

    public long[] getPrimitiveRoots() {
        return primitiveRoots;
    }

    public int getPrimitiveRootCount() {
        return elements;
    }

    public int getRootExponent(Z2Polynomial root) {
        assert root.degree() < exponent;
        return primitiveRootsLog[(int) root.getCoefficients()];
    }

    public Z2Polynomial multiplyRoots(Z2Polynomial multiplier, Z2Polynomial multiplicand) {
        assert multiplier.degree() < exponent && multiplicand.degree() < exponent;
        if (multiplier.isZero() || multiplicand.isZero()) return new Z2Polynomial(0);

        int alphaExponent = primitiveRootsLog[(int) multiplier.getCoefficients()]
                + primitiveRootsLog[(int) multiplicand.getCoefficients()];
        return new Z2Polynomial(primitiveRoots[alphaExponent % primitiveRoots.length]);
    }

    public Z2Polynomial powerOfRoot(Z2Polynomial root, int exponent) {
        assert root.degree() < this.exponent;

        int alphaExponent = primitiveRootsLog[(int) root.getCoefficients()];
        return new Z2Polynomial(primitiveRoots[(alphaExponent * exponent) % primitiveRoots.length]);
    }

    public Z2Polynomial powerOfX(int exponent) {
        return new Z2Polynomial(primitiveRoots[exponent % primitiveRoots.length]);
    }

    public Z2Polynomial rootInverse(Z2Polynomial root) {
        assert !root.isZero(); //There is no multiplicative inverse to 0

        int alphaExponent = primitiveRootsLog[(int) root.getCoefficients()];
        return new Z2Polynomial(primitiveRoots[(elements - alphaExponent) % primitiveRoots.length]);
    }

    public Z2Polynomial calculateSyndrome(Z2Polynomial polynomial, int n) {
        Z2Polynomial result = new Z2Polynomial();
        if (polynomial.getExponentValues() == null)
            return null;

        for (int exponent : polynomial.getExponentValues()) result = result.add(powerOfX(exponent * n));
        return result;
    }
}
