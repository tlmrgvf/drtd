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

package de.tlmrgvf.drtd.dsp.component.biquad;

import de.tlmrgvf.drtd.utils.Complex;

public final class ComplexBiquadFilter extends GenericBiquadFilter<Complex> {
    public ComplexBiquadFilter(Type type, int sampleRate, float center, float qbws, float gain) {
        super(new Complex(0, 0), type, sampleRate, center, qbws, gain);
    }

    public ComplexBiquadFilter(Type type, int sampleRate, float center, float qbws) {
        super(new Complex(0, 0), type, sampleRate, center, qbws);
    }

    public ComplexBiquadFilter(Type type, int sampleRate, float center) {
        super(new Complex(0, 0), type, sampleRate, center);
    }

    @Override
    protected Complex multiply(Complex multiplier, float multiplicand) {
        return multiplier.scale(multiplicand);
    }

    @Override
    protected Complex add(Complex to, Complex amount) {
        return to.add(amount);
    }

    @Override
    protected Complex subtract(Complex from, Complex amount) {
        return from.subtract(amount);
    }
}
