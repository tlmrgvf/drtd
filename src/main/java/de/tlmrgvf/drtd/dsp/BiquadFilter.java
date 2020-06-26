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

package de.tlmrgvf.drtd.dsp;

public final class BiquadFilter {
    /*
     * Formulas: https://www.w3.org/2011/audio/audio-eq-cookbook.html
     * Good explanation of biquad filters: https://arachnoid.com/BiQuadDesigner/index.html
     *                                                      and
     *                                     https://www.earlevel.com/main/2003/02/28/biquads/
     */
    public final static double INVSQRT = 1 / Math.sqrt(2);
    private final int sampleRate;
    private Coefficents coefficients;
    private double z2;
    private double z1;
    private double center;
    private double qbws;
    private double gain;
    private Type type;

    public BiquadFilter(Type type, int sampleRate, double center, double qbws, double gain) {
        this.sampleRate = sampleRate;
        update(type, center, qbws, gain);
    }

    public BiquadFilter(Type type, int sampleRate, double center, double qbws) {
        this(type, sampleRate, center, qbws, 1);
    }

    public BiquadFilter(Type type, int sampleRate, float center) {
        this(type, sampleRate, center, INVSQRT, 1);
    }

    public int getSampleRate() {
        return sampleRate;
    }

    public Coefficents getCoefficients() {
        return new Coefficents(coefficients);
    }

    public void update(Type type, double center, double qbws, double gain) {
        coefficients = type.calculate(sampleRate, center, qbws, gain).normalize();
        this.center = center;
        this.qbws = qbws;
        this.gain = gain;
        this.type = type;
    }

    public void update(Type type, double center, double qbws) {
        update(type, center, qbws, 1);
    }

    public void update(Type type, double center) {
        update(type, center, INVSQRT);
    }

    public Type getType() {
        return type;
    }

    public double getCenter() {
        return center;
    }

    public double getQbws() {
        return qbws;
    }

    public double getGain() {
        return gain;
    }

    public float filterSample(float input) {
        double result = input * coefficients.input0 + z1;
        z1 = input * coefficients.input1 + z2 - result * coefficients.feedback1;
        z2 = input * coefficients.input2 - result * coefficients.feedback2;
        return (float) result;
    }

    public enum Type {

        /**
         * Uses Q factor
         */
        LOWPASS((ret, A, cosomega0, alphaQ, alphaBW, alphaS, omega0) -> {
            ret.input0 = ret.input2 = (1 - cosomega0) / 2;
            ret.input1 = 1 - cosomega0;
            ret.feedback0 = 1 + alphaQ;
            ret.feedback1 = -2 * cosomega0;
            ret.feedback2 = 1 - alphaQ;
            return ret;
        }, "Lowpass"),
        /**
         * Uses Q factor
         */
        HIGHPASS((ret, A, cosomega0, alphaQ, alphaBW, alphaS, omega0) -> {
            ret.input0 = ret.input2 = (1 + cosomega0) / 2;
            ret.input1 = -1 - cosomega0;
            ret.feedback0 = 1 + alphaQ;
            ret.feedback1 = -2 * cosomega0;
            ret.feedback2 = 1 - alphaQ;
            return ret;
        }, "Highpass"),
        /**
         * Uses bandwidth
         */
        BANDPASS_CONSTANT_SKIRT_GAIN((ret, A, cosomega0, alphaQ, alphaBW, alphaS, omega0) -> {
            ret.input0 = Math.sin(omega0) / 2;
            ret.input1 = 0;
            ret.input2 = -ret.input0;
            ret.feedback0 = 1 + alphaBW;
            ret.feedback1 = -2 * cosomega0;
            ret.feedback2 = 1 - alphaBW;
            return ret;
        }, "Bandpass (Constant skirt gain)", false, "Bandwidth"),
        /**
         * Uses bandwidth
         */
        BANDPASS_CONSTANT_PEAK_GAIN((ret, A, cosomega0, alphaQ, alphaBW, alphaS, omega0) -> {
            ret.input0 = alphaBW;
            ret.input1 = 0;
            ret.input2 = -alphaBW;
            ret.feedback0 = 1 + alphaBW;
            ret.feedback1 = -2 * cosomega0;
            ret.feedback2 = 1 - alphaBW;
            return ret;
        }, "Bandpass (Constant peak gain)", false, "Bandwidth"),
        /**
         * Uses bandwidth
         */
        NOTCH((ret, A, cosomega0, alphaQ, alphaBW, alphaS, omega0) -> {
            ret.input0 = 1;
            ret.input1 = -2 * cosomega0;
            ret.input2 = 1;
            ret.feedback0 = 1 + alphaBW;
            ret.feedback1 = -2 * cosomega0;
            ret.feedback2 = 1 - alphaBW;
            return ret;
        }, "Notch", false, "Bandwidth"),
        /**
         * Uses shelf slope
         */
        LOW_SHELF((ret, A, cosomega0, alphaQ, alphaBW, alphaS, omega0) -> {
            ret.input0 = A * ((A + 1) - (A - 1) * cosomega0 + 2 * Math.sqrt(A) * alphaS);
            ret.input1 = 2 * A * ((A - 1) - (A + 1) * cosomega0);
            ret.input2 = A * ((A + 1) - (A - 1) * cosomega0 - 2 * Math.sqrt(A) * alphaS);
            ret.feedback0 = (A + 1) + (A - 1) * cosomega0 + 2 * Math.sqrt(A) * alphaS;
            ret.feedback1 = -2 * ((A - 1) + (A + 1) * cosomega0);
            ret.feedback2 = (A + 1) + (A - 1) * cosomega0 - 2 * Math.sqrt(A) * alphaS;
            return ret;
        }, "Low shelf", true, "Slope"),
        /**
         * Uses shelf slope
         */
        HIGH_SHELF((ret, A, cosomega0, alphaQ, alphaBW, alphaS, omega0) -> {
            ret.input0 = A * ((A + 1) + (A - 1) * cosomega0 + 2 * Math.sqrt(A) * alphaS);
            ret.input1 = -2 * A * ((A - 1) + (A + 1) * cosomega0);
            ret.input2 = A * ((A + 1) + (A - 1) * cosomega0 - 2 * Math.sqrt(A) * alphaS);
            ret.feedback0 = (A + 1) - (A - 1) * cosomega0 + 2 * Math.sqrt(A) * alphaS;
            ret.feedback1 = 2 * ((A - 1) - (A + 1) * cosomega0);
            ret.feedback2 = (A + 1) - (A - 1) * cosomega0 - 2 * Math.sqrt(A) * alphaS;
            return ret;
        }, "High shelf", true, "Slope");

        private final CoefficentCalculator calculator;
        private final String name;
        private final boolean usesGain;
        private final String qbwsLabel;

        Type(CoefficentCalculator calculator, String name) {
            this(calculator, name, false, "Q");
        }

        Type(CoefficentCalculator calculator, String name, boolean usesGain, String qbwsLabel) {
            this.calculator = calculator;
            this.name = name;
            this.usesGain = usesGain;
            this.qbwsLabel = qbwsLabel;
        }

        private Coefficents calculate(int sampleRate, double center, double qbws, double gain) {
            double w0 = 2 * Math.PI * (center / sampleRate);
            double wsin = Math.sin(w0);
            double A = Math.pow(10, gain / 40);
            return calculator.calculate(new Coefficents(sampleRate),
                    A,
                    Math.cos(w0),
                    wsin / (2 * qbws),
                    wsin * Math.sinh(Math.log(2) / 2 * qbws * (w0 / wsin)),
                    wsin / 2 * Math.sqrt((A + 1 / A) * (1 / qbws - 1) + 2),
                    w0);
        }

        public boolean usesGain() {
            return usesGain;
        }

        public String getQbwsLabel() {
            return qbwsLabel;
        }

        @Override
        public String toString() {
            return name;
        }

        interface CoefficentCalculator {
            Coefficents calculate(Coefficents ret,
                                  double A,
                                  double cosomega0,
                                  double alphaQ,
                                  double alphaBW,
                                  double alphaS,
                                  double omega0);
        }
    }

    public final static class Coefficents {
        private final int sampleRate;
        public double feedback0;
        public double feedback1;
        public double feedback2;
        public double input0;
        public double input1;
        public double input2;

        public Coefficents(int sampleRate) {
            this.sampleRate = sampleRate;
        }

        public Coefficents(Coefficents coefficents) {
            this.sampleRate = coefficents.sampleRate;
            this.feedback0 = coefficents.feedback0;
            this.feedback1 = coefficents.feedback1;
            this.feedback2 = coefficents.feedback2;
            this.input0 = coefficents.input0;
            this.input1 = coefficents.input1;
            this.input2 = coefficents.input2;
        }

        public Coefficents normalize() {
            feedback1 /= feedback0;
            feedback2 /= feedback0;
            input1 /= feedback0;
            input2 /= feedback0;
            input0 /= feedback0;
            feedback0 /= feedback0;
            return this;
        }

        public float frequencyResponse(float frequency) {
            double phi = Math.sin(2 * Math.PI * frequency / (2F * sampleRate));
            phi *= phi;

            final double insum = input0 + input1 + input2;
            final double feedbackSum = feedback1 + feedback2 + 1;
            final double fsqr = (16 * input0 * input2 * phi * phi + insum * insum - 4 *
                    (input0 * input1 + 4 * input0 * input2 + input1 * input2) * phi) /
                    (16 * feedback2 * phi * phi + feedbackSum * feedbackSum - 4 *
                            (feedback1 * feedback2 + feedback1 + 4 * feedback2) * phi);
            return (float) (fsqr < 0 ? 0 : Math.sqrt(fsqr));
        }

        @Override
        public String toString() {
            return "Coefficents{" +
                    "sampleRate=" + sampleRate +
                    ", feedback0=" + feedback0 +
                    ", feedback1=" + feedback1 +
                    ", feedback2=" + feedback2 +
                    ", input0=" + input0 +
                    ", input1=" + input1 +
                    ", input2=" + input2 +
                    '}';
        }
    }
}
