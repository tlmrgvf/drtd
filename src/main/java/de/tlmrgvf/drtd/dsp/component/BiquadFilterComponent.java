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
import de.tlmrgvf.drtd.dsp.component.biquad.GenericBiquadFilter;
import de.tlmrgvf.drtd.gui.dialog.BiquadFilterDialog;
import de.tlmrgvf.drtd.utils.Utils;

import java.awt.*;

public final class BiquadFilterComponent extends PipelineComponent<Float, Float> {
    private final static Dimension SIZE = new Dimension(60, 45);
    private static BiquadFilterDialog dialog;
    private final GenericBiquadFilter.Type type;
    private final float center;
    private final float qbws;
    private final float gain;
    private BiquadFilter filter;

    public BiquadFilterComponent(GenericBiquadFilter.Type type, float center) {
        this(type, center, GenericBiquadFilter.INVSQRT);
    }

    public BiquadFilterComponent(GenericBiquadFilter.Type type, float center, float qbws) {
        this(type, center, qbws, 1);
    }

    public BiquadFilterComponent(GenericBiquadFilter.Type type, float center, float qbws, float gain) {
        super(Float.class);
        this.type = type;
        this.center = center;
        this.qbws = qbws;
        this.gain = gain;
    }

    public static void closeDialog() {
        if (dialog != null)
            dialog.dispose();
    }

    @Override
    public void showConfigureDialog() {
        Point location = null;
        if (dialog != null) {
            dialog.dispose();
            location = dialog.getLocation();
        }

        dialog = new BiquadFilterDialog(filter);
        dialog.setVisible(true);
        if (location != null) dialog.setLocation(location);
    }

    @Override
    protected int onInit(int calculatedInputSampleRate) {
        filter = new BiquadFilter(type, getInputSampleRate(), center, qbws, gain);
        return calculatedInputSampleRate;
    }

    @Override
    protected synchronized Float calculate(Float input) {
        return filter.filterSample(input);
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        return SIZE;
    }

    @Override
    protected void drawRelative(Graphics2D graphics) {
        graphics.drawRect(0, 0, SIZE.width - 1, SIZE.height - 1);
        graphics.translate(8, 8);
        Dimension drawSize = Utils.resize(SIZE, -16, -16);
        final int oneThirdWidth = drawSize.width / 3;
        final int twoThirdWidth = 2 * oneThirdWidth;
        final int halfWidth = drawSize.width / 2;

        switch (filter.getType()) {
            case LOWPASS:
                Utils.drawPath(graphics, 0, 0, oneThirdWidth, 0, halfWidth, drawSize.height);
                break;
            case HIGHPASS:
                Utils.drawPath(graphics, halfWidth, drawSize.height, twoThirdWidth, 0, drawSize.width, 0);
                break;
            case LOW_SHELF:
                Utils.drawPath(graphics,
                        0, 0,
                        oneThirdWidth, 0,
                        twoThirdWidth, drawSize.height,
                        drawSize.width, drawSize.height);
                break;
            case HIGH_SHELF:
                Utils.drawPath(graphics,
                        0, drawSize.height,
                        oneThirdWidth, drawSize.height,
                        twoThirdWidth, 0,
                        drawSize.width, 0);
                break;
            case BANDPASS_CONSTANT_PEAK_GAIN:
                graphics.drawLine(0, 0, drawSize.width, 0);
                Utils.drawPath(graphics,
                        oneThirdWidth, drawSize.height,
                        halfWidth, 0,
                        twoThirdWidth, drawSize.height);
                break;
            case BANDPASS_CONSTANT_SKIRT_GAIN:
                graphics.drawLine(0, 0, drawSize.width, 0);
                Utils.drawPath(graphics,
                        oneThirdWidth, drawSize.height,
                        halfWidth, 10,
                        twoThirdWidth, drawSize.height);
                break;
            case NOTCH:
                Utils.drawPath(graphics,
                        0, 0,
                        oneThirdWidth, 0,
                        halfWidth, drawSize.height,
                        twoThirdWidth, 0,
                        drawSize.width, 0);
                break;
            default:
                assert false;
        }
    }
}
