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

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

public final class Z2Polynomial {
    public final static int NUM_COEFFS = Long.SIZE;
    public final static long MSB_MASK = 0x8000000000000000L;

    private final long coefficients;
    private final int degree;
    private List<Integer> exponents;
    private String stringValue;

    public Z2Polynomial(long coefficients) {
        this.coefficients = coefficients;

        if (coefficients == 0) {
            this.degree = -1;
        } else {
            int deg = NUM_COEFFS - 1;
            int shift = 0;
            while (deg >= 0 && ((coefficients << shift++) & MSB_MASK) == 0) deg--;
            this.degree = deg;
        }
    }

    public Z2Polynomial() {
        this.coefficients = 0;
        this.degree = -1;
    }

    private static Z2Polynomial polyDivide(Z2Polynomial dividend, Z2Polynomial divisor, boolean remainder) {
        long dividing = dividend.coefficients;
        long divisorShift = divisor.coefficients << (dividend.degree - divisor.degree);
        long checkBit = 1 << dividend.degree;
        int steps = (dividend.degree - divisor.degree) + 1;

        int quotientBit = dividend.degree - divisor.degree;
        long quotient = 0;

        for (int i = 0; i < steps; ++i) {
            if ((dividing & checkBit) != 0) {
                quotient |= 1 << quotientBit;
                dividing ^= divisorShift;
            }
            divisorShift >>= 1;
            checkBit >>= 1;
            quotientBit--;
        }

        return new Z2Polynomial(remainder ? dividing : quotient);
    }

    public long getCoefficients() {
        return coefficients;
    }

    public final List<Integer> getExponentValues() {
        if (isZero()) return Collections.emptyList();
        if (exponents != null) return exponents; //Avoid recalculations, especially when calling toString()

        long coeffShift = coefficients;
        List<Integer> exponents = new ArrayList<>(NUM_COEFFS);

        for (int i = 0; i <= degree; ++i) {
            if ((coeffShift & 1) == 1) exponents.add(i);
            coeffShift >>= 1;
        }

        this.exponents = exponents;
        return exponents;
    }

    public boolean isZero() {
        return coefficients == 0;
    }

    public Z2Polynomial add(Z2Polynomial toAdd) {
        return new Z2Polynomial(toAdd.coefficients ^ coefficients);
    }

    public Z2Polynomial multiply(Z2Polynomial toMultiply) {
        if (isZero() || toMultiply.isZero())
            return new Z2Polynomial();

        if (degree == 0 || toMultiply.degree == 0)
            return (degree == 0) ? toMultiply : this;

        if (degree == 1 || toMultiply.degree == 1)
            return new Z2Polynomial((degree == 1) ? toMultiply.coefficients << 1 : coefficients << 1);

        assert degree + toMultiply.degree < NUM_COEFFS;

        long result = 0;
        for (int exponent : getExponentValues()) {
            for (int otherExponent : toMultiply.getExponentValues()) {
                result ^= 1 << (exponent + otherExponent);
            }
        }

        return new Z2Polynomial(result);
    }

    public Z2Polynomial remainder(Z2Polynomial divisor) {
        if (divisor.degree > degree) {
            return this;
        }

        return polyDivide(this, divisor, true);
    }

    public Z2Polynomial divide(Z2Polynomial divisor) {
        if (divisor.degree > degree) {
            return new Z2Polynomial();
        }

        return polyDivide(this, divisor, false);
    }

    public int degree() {
        return degree;
    }


    @Override
    public String toString() {
        if (stringValue == null) {
            List<Integer> exponentValues = new ArrayList<>(getExponentValues());
            if (exponentValues == null) {
                stringValue = "0";
            } else {
                Collections.reverse(exponentValues);
                StringBuilder sb = new StringBuilder();
                Iterator<Integer> iterator = exponentValues.iterator();

                while (iterator.hasNext()) {
                    int i = iterator.next();

                    if (i == 0) {
                        sb.append("1");
                    } else {
                        sb.append("x^");
                        sb.append(i);
                        if (iterator.hasNext()) sb.append("+");
                    }
                }

                stringValue = sb.toString();
            }

        }

        return "Z2Polynomial{" +
                "coefficents=0b" + Long.toBinaryString(coefficients) +
                ", " + stringValue +
                ", degree=" + degree +
                '}';
    }
}
