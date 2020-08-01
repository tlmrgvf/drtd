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

package de.tlmrgvf.drtd.gui.component;

import de.tlmrgvf.drtd.gui.dialog.ScopeDialog;
import de.tlmrgvf.drtd.gui.utils.Canvas;
import de.tlmrgvf.drtd.gui.utils.Layer;
import de.tlmrgvf.drtd.utils.SettingsManager;
import de.tlmrgvf.drtd.utils.Utils;

import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.ConcurrentLinkedQueue;

public final class Scope extends Canvas {
    private final static SettingsManager SETTINGS_MANAGER = SettingsManager.createFor(Scope.class);

    private final Layer scopeLayer;
    private final AbstractQueue<Float> bufferedValues = new ConcurrentLinkedQueue<>();

    private float[] samples;
    private int width;
    private int samplesCaptured = 0;
    private int samplesAveraged = 0;
    private float sampleAverage;
    private float maxSampleValue = Float.MIN_VALUE;
    private float minSampleValue = Float.MAX_VALUE;
    private float dcBias;
    private float lastSample;

    private int sampleRatio = 1;
    private boolean oversampling;
    private int triggerPosition;
    private boolean paused;
    private boolean singleShot;
    private boolean removeDcBias;
    private boolean freerun;
    private boolean normalize;

    public Scope() {
        final ListenerImpl listener = new ListenerImpl();
        scopeLayer = createLayer(0, 0, Layer.PARENT_SIZE, Layer.PARENT_SIZE);
        width = getWidth();

        addMouseListener(listener);
        setResizeListener((e) -> drawLayers(true));
        setBackground(Color.BLACK);
        samples = new float[width];

        SETTINGS_MANAGER
                .mapOption(Integer.class, this::getSampleRatio, this::setSampleRatio, 1)
                .mapOption(Boolean.class, this::isFreerun, this::setFreerun, true)
                .mapOption(Integer.class, this::getTriggerPosition, this::setTriggerPosition, 50)
                .mapOption(Boolean.class, this::isRemoveDcBias, this::setRemoveDcBias, true)
                .mapOption(Boolean.class, this::isNormalize, this::setNormalize, false)
                .loadAll();
    }

    public boolean isNormalize() {
        return normalize;
    }

    public synchronized void setNormalize(boolean normalize) {
        this.normalize = normalize;
    }

    public int getSampleRatio() {
        return oversampling ? -sampleRatio : sampleRatio;
    }

    public synchronized void setSampleRatio(int sampleRatio) {
        this.sampleRatio = Math.abs(sampleRatio);
        oversampling = sampleRatio < 0;
    }

    public int getTriggerPosition() {
        return triggerPosition;
    }

    public synchronized void setTriggerPosition(int triggerPosition) {
        this.triggerPosition = Utils.clampBetween(0, 100, triggerPosition);
    }

    public boolean isPaused() {
        return paused;
    }

    public synchronized void setPaused(boolean paused) {
        this.paused = paused;
    }

    public boolean isRemoveDcBias() {
        return removeDcBias;
    }

    public synchronized void setRemoveDcBias(boolean removeDcBias) {
        this.removeDcBias = removeDcBias;
    }

    public void captureOnce() {
        singleShot = true;
        samplesCaptured = 0;
    }

    public void addValue(Float sample) {
        bufferedValues.offer(sample);
    }

    public synchronized void processValues() {
        Iterator<Float> iterator = bufferedValues.iterator();
        boolean draw = false;
        while (iterator.hasNext()) {
            draw |= processValue(iterator.next());
            iterator.remove();
        }

        if (draw)
            drawLayers(false);
    }

    private boolean processValue(Float sample) {
        if (paused && !singleShot)
            return false;

        boolean dirty = false;
        if ((!oversampling && samplesCaptured >= width) || (oversampling && samplesCaptured * sampleRatio >= width)) {
            if (samples != null && willDraw()) {
                recalculateBias();
                final float bias = getBias();
                float adjMax;

                if (normalize) {
                    adjMax = 1;
                } else {
                    adjMax = Math.max(maxSampleValue, Math.abs(minSampleValue)) - bias;
                    adjMax *= 1.1D;
                }

                int halfHeight = getHeight() / 2;
                Point lastPoint = null;

                scopeLayer.clear();
                Graphics2D scopeGraphics = scopeLayer.getGraphics();
                scopeGraphics.setColor(Color.GRAY);
                scopeGraphics.drawLine(0, getHeight() / 2, getWidth(), getHeight() / 2);

                if (!freerun) {
                    scopeGraphics.setColor(Color.YELLOW);
                    final float normalizedTrigger = ((triggerPosition / 50F) - 1) * 1.1F;
                    final float translatedSample = normalizedTrigger * (maxSampleValue - dcBias)
                            + (removeDcBias ? 0 : dcBias);
                    final int y = (int) (translatedSample / adjMax * -halfHeight);

                    scopeGraphics.drawLine(0, y + halfHeight, getWidth(), y + halfHeight);
                }

                scopeGraphics.setColor(Color.RED);
                if (adjMax > 0) {
                    int x = 0;
                    for (float s : samples) {
                        s -= bias;
                        s /= adjMax;
                        s *= -halfHeight;

                        Point p = new Point(x, (int) (s + halfHeight));
                        if (lastPoint == null) lastPoint = p;

                        scopeGraphics.drawLine(lastPoint.x, lastPoint.y, lastPoint.x, p.y);
                        scopeGraphics.drawLine(lastPoint.x, p.y, p.x, p.y);

                        if ((lastPoint.x + sampleRatio) >= width && oversampling) break;

                        lastPoint = p;
                        ++x;
                        if (oversampling) x += sampleRatio;
                    }
                }
                dirty = true;
            }

            samplesCaptured = 0;
            singleShot = false;
        }

        if (samplesCaptured == 0) {
            recalculateBias();
            maxSampleValue = Math.max(maxSampleValue, sample);
            minSampleValue = Math.min(minSampleValue, sample);
            int width = getWidth();
            if (this.width != width) {
                this.width = width;
                samples = new float[this.width];
            }

            if (maxSampleValue != 0) {
                final float dBias = maxSampleValue - dcBias;
                final float nxt = (sample - dcBias) / dBias;
                final float old = (lastSample - dcBias) / dBias;
                final float p = ((triggerPosition / 50F) - 1) * 1.1F;

                if (!(old < p && nxt >= p) && !freerun) {
                    lastSample = sample;
                    return false;
                }
            }
            maxSampleValue = Float.MIN_VALUE;
            minSampleValue = Float.MAX_VALUE;
        }

        if (sampleRatio == 0 || oversampling) {
            samples[samplesCaptured++] = sample;
            maxSampleValue = Math.max(maxSampleValue, sample);
            minSampleValue = Math.min(minSampleValue, sample);
        } else if (samplesAveraged > sampleRatio) {
            float avgSample = sampleAverage / (float) (sampleRatio + 1);
            samples[samplesCaptured++] = avgSample;
            maxSampleValue = Math.max(maxSampleValue, avgSample);
            minSampleValue = Math.min(minSampleValue, avgSample);
            samplesAveraged = 0;
            sampleAverage = 0;
        } else {
            ++samplesAveraged;
            sampleAverage += sample;
        }

        lastSample = sample;
        return dirty;
    }

    private float getBias() {
        return removeDcBias ? dcBias : 0;
    }

    private void recalculateBias() {
        dcBias = minSampleValue + (maxSampleValue - minSampleValue) / 2F;
    }

    public boolean isFreerun() {
        return freerun;
    }

    public synchronized void setFreerun(boolean freerun) {
        this.freerun = freerun;
    }

    private class ListenerImpl extends MouseAdapter {
        @Override
        public void mouseClicked(MouseEvent mouseEvent) {
            if (mouseEvent.getButton() == MouseEvent.BUTTON3) {
                ScopeDialog dialog = ScopeDialog.getInstance(Scope.this);
                dialog.setState(Frame.NORMAL);
                dialog.setVisible(true);
                dialog.loadFromScope();
            }
        }
    }
}
