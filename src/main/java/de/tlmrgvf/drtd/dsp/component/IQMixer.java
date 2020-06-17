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
import de.tlmrgvf.drtd.gui.utils.config.ConfigureDialog;
import de.tlmrgvf.drtd.gui.utils.config.FloatSetting;
import de.tlmrgvf.drtd.utils.Complex;

import java.awt.*;


public final class IQMixer extends PipelineComponent<Float, Complex> {
    private final static int ICON_RADIUS = 11;
    private final static Dimension SIZE = new Dimension(ICON_RADIUS * 2 - 1, ICON_RADIUS * 2 - 1);
    private double phase = 0;
    private double pStep;
    private float frequency;

    public IQMixer(float frequency) {
        super(Complex.class);
        this.frequency = frequency;
    }

    public float getFrequency() {
        return frequency;
    }

    public synchronized void setFrequency(float frequency) {
        this.frequency = frequency;
        onInit(getInputSampleRate());
    }

    @Override
    public void showConfigureDialog() {
        ConfigureDialog.showDialog(IQMixer.class,
                "IQ mixer",
                new FloatSetting("Frequency (Hz)",
                        this::getFrequency,
                        this::setFrequency,
                        0F,
                        (float) getInputSampleRate(),
                        1F));
    }

    @Override
    public int onInit(int calculatedInputSampleRate) {
        this.pStep = (float) (2 * Math.PI / (float) calculatedInputSampleRate * frequency);
        return calculatedInputSampleRate;
    }

    @Override
    protected synchronized Complex calculate(Float sample) {
        phase += pStep;
        phase %= Math.PI * 2;

        return new Complex(sample * (float) Math.sin(phase), sample * (float) Math.cos(phase));
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        return SIZE;
    }

    @Override
    public void drawRelative(Graphics2D graphics) {
        graphics.drawOval(0, 0, SIZE.width - 1, SIZE.height - 1);
        final int cornerFromCenter = (int) Math.round(Math.sqrt((ICON_RADIUS * ICON_RADIUS) / 2F) - 1);
        graphics.translate(ICON_RADIUS - 1, ICON_RADIUS - 1);
        graphics.drawLine(0, 0, cornerFromCenter, cornerFromCenter);
        graphics.drawLine(0, 0, cornerFromCenter, -cornerFromCenter);
        graphics.drawLine(0, 0, -cornerFromCenter, cornerFromCenter);
        graphics.drawLine(0, 0, -cornerFromCenter, -cornerFromCenter);
    }
}
