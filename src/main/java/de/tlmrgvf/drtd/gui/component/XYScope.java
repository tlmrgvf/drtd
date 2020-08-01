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

import de.tlmrgvf.drtd.gui.utils.Canvas;
import de.tlmrgvf.drtd.gui.utils.Layer;
import de.tlmrgvf.drtd.utils.Complex;

import java.awt.*;

public final class XYScope extends Canvas {
    private final boolean connect;
    private final Complex[] values;
    private final float fadeFactor;
    private final float drawFactor;
    private final Layer scopeLayer;
    private int valuesCollected;
    private float maxMagnitude;

    public XYScope(int size, boolean connect, int historySize, float fadeFactor, float drawFactor) {
        values = new Complex[historySize];
        this.connect = connect;
        this.fadeFactor = fadeFactor;
        this.drawFactor = drawFactor;
        scopeLayer = createLayer(0, 0, Layer.PARENT_SIZE, Layer.PARENT_SIZE);
        setBackground(Color.BLACK);

        Dimension dim = new Dimension(size, size);
        setMaximumSize(dim);
        setMinimumSize(dim);
        setPreferredSize(dim);
    }

    public synchronized void process(Complex value) {
        values[valuesCollected++] = value;
        maxMagnitude = Math.max(maxMagnitude, value.magnitudeSquared());

        if (valuesCollected == values.length) {
            if (willDraw()) {
                final Graphics g = scopeLayer.getGraphics();
                final int width = getWidth();
                final int height = getHeight();
                g.setColor(new Color(0, 0, 0, fadeFactor));
                g.fillRect(0, 0, width, height);
                g.setColor(new Color(0, 1F, 0, drawFactor));

                final float factor = (float) (1 / Math.sqrt(maxMagnitude));

                final int xOffset = width / 2;
                final int yOffset = height / 2;

                final float scale = Math.min(width, height) * .48F;

                Point previous = null;
                for (Complex toDraw : values) {
                    final Complex scaled = toDraw.scale(factor);
                    final Point next = new Point((int) (scaled.real * scale) + xOffset,
                            (int) (scaled.imag * scale) + yOffset);

                    if (connect) {
                        if (previous == null) {
                            previous = next;
                            continue;
                        }

                        g.drawLine(previous.x, previous.y, next.x, next.y);
                        previous = next;
                    } else {
                        g.fillRect(next.x, next.y, 1, 1);
                    }
                }

                drawLayers(false);
            }

            valuesCollected = 0;
            maxMagnitude = 0;
        }
    }
}
