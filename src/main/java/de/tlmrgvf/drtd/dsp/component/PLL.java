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

package de.tlmrgvf.drtd.dsp.component;

import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.dsp.component.biquad.BiquadFilter;
import de.tlmrgvf.drtd.dsp.component.biquad.ComplexBiquadFilter;
import de.tlmrgvf.drtd.gui.dialog.PLLDialog;
import de.tlmrgvf.drtd.utils.Complex;

import java.awt.*;

public final class PLL extends PipelineComponent<Float, Complex> {
    /*
     * https://arachnoid.com/phase_locked_loop/index.html
     */

    private static PLLDialog dialog;
    private final BiquadFilter loopFilter;
    private final ComplexBiquadFilter lowpassFilter;
    private final PhaseDetector detector;
    private float frequency;
    private float loopGain;
    private float phase;
    private float phaseStep;

    public PLL(BiquadFilter loopFilter,
               ComplexBiquadFilter lowpassFilter,
               PhaseDetector detector,
               float frequency,
               float loopGain) {
        super(Complex.class);
        this.loopFilter = loopFilter;
        this.lowpassFilter = lowpassFilter;
        this.detector = detector;
        this.frequency = frequency;
        this.loopGain = loopGain;
    }

    public static void closeDialog() {
        if (dialog != null)
            dialog.dispose();
    }

    @Override
    protected int onInit(int calculatedInputSampleRate) {
        phaseStep = (float) (2 * Math.PI / getInputSampleRate() * frequency);
        return calculatedInputSampleRate;
    }

    @Override
    public void showConfigureDialog() {
        Point location = null;
        if (dialog != null) {
            dialog.dispose();
            location = dialog.getLocation();
        }

        dialog = new PLLDialog(this);
        dialog.setVisible(true);
        if (location != null) dialog.setLocation(location);
    }

    public BiquadFilter getLoopFilter() {
        return loopFilter;
    }

    public ComplexBiquadFilter getLowpassFilter() {
        return lowpassFilter;
    }

    public float getFrequency() {
        return frequency;
    }

    public void setFrequency(float frequency) {
        phaseStep = (float) (2 * Math.PI / getInputSampleRate() * frequency);
        this.frequency = frequency;
    }

    public float getLoopGain() {
        return loopGain;
    }

    public void setLoopGain(float loopGain) {
        this.loopGain = loopGain;
    }

    @Override
    protected Complex calculate(Float input) {
        float i = input * (float) Math.sin(phase);
        float q = input * (float) Math.cos(phase);

        final Complex result = lowpassFilter.filterSample(new Complex(i, q).normalize());
        final float phaseError = loopFilter.filterSample(detector.detect(result) * loopGain);
        phase += (float) (phaseStep + 2 * Math.PI * frequency * phaseError / (float) getInputSampleRate());
        phase %= 2 * Math.PI;
        if (dialog != null && dialog.isVisible())
            dialog.updateDialog(phaseError);

        return result;
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        return new Dimension(10, 10);
    }

    @Override
    protected void drawRelative(Graphics2D graphics) {

    }

    public interface PhaseDetector {
        float detect(Complex input);
    }
}
