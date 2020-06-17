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

package de.tlmrgvf.drtd.dsp.component.movingaverage;

import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.gui.utils.config.ConfigureDialog;
import de.tlmrgvf.drtd.gui.utils.config.IntegerSetting;
import de.tlmrgvf.drtd.utils.Utils;
import de.tlmrgvf.drtd.utils.structure.GenericRingBuffer;

import java.awt.*;
import java.awt.font.LineMetrics;

public abstract class GenericMovingAverage<T> extends PipelineComponent<T, T> {
    private final static String LABEL = "x";
    private final static Font FONT = Utils.FONT.deriveFont(12F);
    private final T zero;

    private GenericRingBuffer<T> buffer;
    private int taps;
    private int zeroes;
    private T average;
    private Rectangle bounds;
    private LineMetrics lineMetrics;
    private Dimension dimension;

    public GenericMovingAverage(Class<T> inputClass, T zero, int taps) {
        super(inputClass);
        if (taps < 1)
            throw new IllegalStateException("Invalid amount of taps!");

        this.zero = zero;
        this.taps = taps;
        average = zero;
        buffer = new GenericRingBuffer<>(this::createArray, zero, taps);
    }

    protected abstract T[] createArray(int size);

    protected abstract T subtract(T from, T amount);

    protected abstract T add(T to, T amount);

    protected abstract T divide(T toDivide, int amount);

    protected abstract boolean isZero(T toCheck);

    @Override
    protected synchronized T calculate(T sample) {
        if (taps == 1) return sample;
        average = add(average, subtract(sample, buffer.push(sample)));

        /* Fixes problems with floating point inaccuracies by just forcing average to 0 */
        if (isZero(sample)) {
            if (zeroes >= taps)
                average = zero;
            else
                ++zeroes;
        } else {
            zeroes = 0;
        }

        return divide(average, taps);
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        if (dimension != null) return dimension;

        g.setFont(FONT);
        bounds = g.getFont().getStringBounds(LABEL, g.getFontRenderContext()).getBounds();
        lineMetrics = g.getFont().getLineMetrics(LABEL, g.getFontRenderContext());
        dimension = new Dimension(bounds.width + 10, bounds.height + 10);
        return dimension;
    }

    public synchronized void setTaps(int taps) {
        this.taps = taps;
        buffer = new GenericRingBuffer<>(this::createArray, zero, taps);
        average = zero;
        ConfigureDialog.updateDialog(GenericMovingAverage.class);
    }

    public int getTaps() {
        return taps;
    }

    @Override
    public void showConfigureDialog() {
        ConfigureDialog.showDialog(GenericMovingAverage.class,
                "Moving average",
                new IntegerSetting("Taps", this::getTaps, this::setTaps, 1, 5000, 1));
    }

    @Override
    public void drawRelative(Graphics2D g) {
        Dimension dim = calculateSize(g);
        g.setFont(FONT);
        int xOff = Utils.center(dim.width, bounds.width);
        g.drawRect(0, 0, dim.width - 1, dim.height - 1);
        g.drawString(LABEL, xOff, -lineMetrics.getStrikethroughOffset() + bounds.height);

        g.fillRect(xOff, bounds.height / 2, (int) bounds.getWidth(), 2);
    }
}
