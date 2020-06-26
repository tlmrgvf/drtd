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

package de.tlmrgvf.drtd.dsp.component.firfilter;

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.dsp.window.Window;
import de.tlmrgvf.drtd.gui.dialog.FilterDialog;
import de.tlmrgvf.drtd.utils.Utils;
import de.tlmrgvf.drtd.utils.structure.GenericRingBuffer;

import java.awt.*;
import java.awt.geom.GeneralPath;
import java.util.logging.Logger;

public abstract class GenericFirFilter<T> extends PipelineComponent<T, T> {
    public final static int MAX_TAPS = 8192;
    private final static Logger LOGGER = Drtd.getLogger(GenericFirFilter.class);
    private final static Dimension SIZE = new Dimension(60, 45);
    private static FilterDialog dialog;
    private final T zero;

    private Window window;
    private GenericRingBuffer<T> buffer;
    private float[] coefficients;
    private int taps;
    private boolean bandStop;
    private int fStart, fStop;

    public GenericFirFilter(Class<T> contentClass, T zero, Window window, int taps, int fStart, int fStop) {
        super(contentClass);
        if (taps < 1 || taps > MAX_TAPS)
            throw new IllegalArgumentException("Invalid amount of taps!");

        if (taps % 2 == 0) {
            LOGGER.warning("Number of taps is even, subtracting 1!");
            taps--;
        }

        this.zero = zero;
        this.taps = taps;
        this.window = window;
        this.fStart = fStart;
        this.fStop = fStop;
        this.buffer = new GenericRingBuffer<>(this::createArray, zero, taps);
    }

    public static void closeDialog() {
        if (dialog != null)
            dialog.dispose();
    }

    protected abstract T[] createArray(int size);

    protected abstract T multiply(float multiplier, T multiplicand);

    protected abstract T add(T to, T amount);

    @Override
    public int onInit(int calculatedInputSampleRate) {
        recalculateFilterCoefficients();
        return calculatedInputSampleRate;
    }

    @Override
    protected synchronized T calculate(T sample) {
        if (taps == 1) return sample;

        buffer.push(sample);

        T sum = zero;
        for (int i = 0; i < taps; ++i)
            sum = add(sum, multiply(coefficients[taps - i - 1], buffer.peek(i)));

        return sum;
    }

    @Override
    public void showConfigureDialog() {
        Point location = null;
        if (dialog != null) {
            dialog.dispose();
            location = dialog.getLocation();
        }

        dialog = new FilterDialog(this);
        dialog.setVisible(true);
        if (location != null) dialog.setLocation(location);
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        return SIZE;
    }

    private void drawSine(Graphics2D g, int index, boolean strikethrough) {
        final float width = SIZE.width * .7F;
        final float height = SIZE.height * .7F;
        final float sinHeight = height / 6;
        final float yOffset = Utils.center(SIZE.height, (int) height) + 2 * sinHeight * index + sinHeight;
        final float xOffset = Utils.center(SIZE.width, (int) width);

        var path = new GeneralPath();
        path.moveTo(xOffset, yOffset);
        for (int xi = 1; xi < width; ++xi) {
            final double yi = sinHeight * Math.sin(2 * Math.PI * (xi / width)) + yOffset;
            path.lineTo(xi + xOffset, yi);
        }
        g.draw(path);

        if (strikethrough)
            g.drawLine(SIZE.width / 2 - 2, (int) yOffset - 2, SIZE.width / 2 + 2, (int) yOffset + 2);
    }

    @Override
    public void drawRelative(Graphics2D g) {
        g.drawRect(0, 0, SIZE.width - 1, SIZE.height - 1);
        g.setStroke(new BasicStroke(1.6F, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));

        final boolean removeLow = fStart > 0 ^ bandStop;
        final boolean removeMid = fStop < getInputSampleRate() / 6 ^ bandStop;
        final boolean removeHigh = fStop < getInputSampleRate() / 2 ^ bandStop;

        drawSine(g, 0, removeLow);
        drawSine(g, 1, removeMid);
        drawSine(g, 2, removeHigh);
    }

    public synchronized void recalculateFilterCoefficients() {
        final float[] windowCoeffs = this.window.calculateCoefficients(taps);
        final float[] coeffs = new float[taps];
        final int center = (taps - 1) / 2;
        final float sampleRate = getInputSampleRate();

        if (fStart == fStop) {
            for (int i = 0; i < taps; ++i) {
                coeffs[i] = windowCoeffs[i] * (float) Math.sin(2 * Math.PI * i / (sampleRate / (float) fStart));
            }
        } else {
            coeffs[center] = 2 * (fStop - fStart) / sampleRate;
            if (bandStop) coeffs[center] = 1 - coeffs[center];

            for (int i = center + 1; i < taps; ++i) {
                final int norm = i - center;
                final float tip = 2 * norm * (float) Math.PI;
                final int j = taps - i - 1;
                float val = windowCoeffs[i] * (float) ((Math.sin(tip * (fStop / sampleRate)) - Math.sin(tip * (fStart / sampleRate))) / (norm * Math.PI));

                if (bandStop) val = -val;
                coeffs[i] = val;
                coeffs[j] = val;
            }
        }

        coefficients = coeffs;

        if (dialog != null)
            dialog.updateFromFilter();
    }

    public Window getWindow() {
        return window;
    }

    public synchronized void setWindow(Window window) {
        this.window = window;
        recalculateFilterCoefficients();
    }

    public float[] getCoefficients() {
        return coefficients;
    }

    public int getTaps() {
        return taps;
    }

    public synchronized void setTaps(int taps) {
        if (taps < 1 || taps > MAX_TAPS)
            throw new IllegalArgumentException("Invalid amount of taps!");

        this.taps = taps;
        this.buffer = new GenericRingBuffer<>(this::createArray, zero, taps);
        recalculateFilterCoefficients();
    }

    public boolean isBandStop() {
        return bandStop;
    }

    public void setBandStop(boolean bandStop) {
        this.bandStop = bandStop;
        recalculateFilterCoefficients();
    }

    public int getStopFrequency() {
        return fStop;
    }

    public void setStopFrequency(int fStop) {
        if (fStop < fStart)
            throw new IllegalArgumentException("fStop < fStart!");

        this.fStop = fStop;
        recalculateFilterCoefficients();
    }

    public int getStartFrequency() {
        return fStart;
    }

    public void setStartFrequency(int fStart) {
        if (fStop < fStart)
            throw new IllegalArgumentException("fStop < fStart!");

        this.fStart = fStart;
        recalculateFilterCoefficients();
    }
}
