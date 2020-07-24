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

package de.tlmrgvf.drtd.dsp.window;

public final class KaiserWindow extends Window {
    private static final long serialVersionUID = 3312418553667924412L;
    private final static double BESSEL_FUNC_APPROX = 1e-11;
    private int attenuation;

    public KaiserWindow(int attenuation) {
        this.attenuation = attenuation;
    }

    public KaiserWindow() {
        this.attenuation = 20;
    }

    public static float I0(double x) {
        float sum = 1, add = 1, fac = 1; //m = 0

        for (int m = 1; add > BESSEL_FUNC_APPROX; ++m) {
            fac *= m;
            add = (1 / (fac * fac)) * (float) Math.pow(x / 2D, 2 * m);
            sum += add;
        }

        return sum;
    }

    public int getAttenuation() {
        return attenuation;
    }

    public void setAttenuation(int attenuation) {
        this.attenuation = attenuation;
    }

    @Override
    public String getDisplayName() {
        return "Kaiser";
    }

    @Override
    public float[] calculateCoefficients(int width) {
        final float alpha;

        if (attenuation < 21) {
            alpha = 0;
        } else if (attenuation > 50) {
            alpha = 0.1102F * (attenuation - 8.7F);
        } else {
            alpha = (float) (0.5842 * Math.pow((attenuation - 21), 0.4) + 0.07886 * (attenuation - 21));
        }

        final int center = (width - 1) / 2;
        final float[] kaiser = new float[width];
        final double i0Alpha = I0(alpha);
        final double centerSqr = center * center;

        for (int i = center; i < width; ++i) {
            final int norm = i - center;
            final float value = (float) (I0(alpha * Math.sqrt(1 - ((norm * norm) / centerSqr))) / i0Alpha);

            //Only have to calculate half of the coefficients, the other half is identical
            kaiser[i] = value;
            kaiser[width - i - 1] = value;
        }
        return kaiser;
    }

}
