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

import de.tlmrgvf.drtd.dsp.Interpreter;

import java.awt.*;
import java.util.Objects;

public final class Complex {
    public final float real, imag;

    public Complex() {
        real = 0;
        imag = 0;
    }

    public Complex(int real, int imag) {
        this.real = real;
        this.imag = imag;
    }

    public Complex(Float real, Float imag) {
        this.real = real == null ? 0 : real;
        this.imag = imag == null ? 0 : imag;
    }

    public float angle() {
        return (float) Math.atan2(imag, real);
    }

    public float magnitude() {
        return (float) Math.sqrt(real * real + imag * imag);
    }

    public float magnitudeSquared() {
        return real * real + imag * imag;
    }

    public static Interpreter createInterpreter() {
        return new Interpreter() {
            private boolean showImaginary;

            @Override
            public Float interpret(Object value) {
                Complex comp = (Complex) value;
                return showImaginary ? comp.imag : comp.real;
            }

            @Override
            public String[] getViewableValues() {
                return new String[]{"Real", "Imaginary"};
            }

            @Override
            public Color getColor() {
                return new Color(.5F, .5F, .8F);
            }

            @Override
            public void view(int i) {
                showImaginary = i == 1;
            }
        };
    }

    public Complex add(Complex toAdd) {
        return new Complex(real + toAdd.real, imag + toAdd.imag);
    }

    public Complex subtract(Complex toSubtract) {
        return new Complex(real - toSubtract.real, imag - toSubtract.imag);
    }

    public Complex multiply(Complex toMultiply) {
        return new Complex(real * toMultiply.real - imag * toMultiply.imag,
                real * toMultiply.imag + toMultiply.real * imag);
    }

    public Complex normalize() {
        float magnitude = magnitude();
        if (magnitude == 0)
            return new Complex();

        return scale(1 / magnitude);
    }

    public Complex scale(float factor) {
        return new Complex(real * factor, imag * factor);
    }

    @Override
    public String toString() {
        return "Complex{" +
                "real=" + real +
                ", imag=" + imag +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass())
            return false;
        Complex complex = (Complex) o;
        return Float.compare(complex.real, real) == 0 &&
                Float.compare(complex.imag, imag) == 0;
    }

    @Override
    public int hashCode() {
        return Objects.hash(real, imag);
    }
}
