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
import de.tlmrgvf.drtd.utils.structure.RingBuffer;

import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;

public final class RollingScope extends Canvas {
    private final Layer valueLayer;
    private final Layer backgroundLayer;
    private final RingBuffer buffer;

    private float maxValue = Float.MIN_VALUE;
    private float minValue = Float.MAX_VALUE;
    private Point previousPoint = null;

    public RollingScope() {
        backgroundLayer = createLayer(0, 0, Layer.PARENT_SIZE, Layer.PARENT_SIZE);
        valueLayer = createLayer(0, 0, Layer.PARENT_SIZE, Layer.PARENT_SIZE);
        buffer = new RingBuffer(1);

        addMouseListener(new MouseAdapter() {
            @Override
            public void mouseClicked(MouseEvent e) {
                if (e.getClickCount() == 2 && e.getButton() == MouseEvent.BUTTON1)
                    clear();
            }
        });

        setResizeListener((e) -> clear());
    }

    private void clear() {
        final var graphics = backgroundLayer.getGraphics();
        graphics.setColor(Color.BLACK);
        graphics.fillRect(0, 0, backgroundLayer.getWidth(), backgroundLayer.getHeight());
        synchronized (RollingScope.this) {
            buffer.resize(valueLayer.getWidth());
        }

        maxValue = Float.MIN_VALUE;
        minValue = Float.MAX_VALUE;

        drawLayers(true);
    }

    private void fullRedraw() {
        if (willDraw()) {
            valueLayer.clear();
            previousPoint = null;

            var graphics = valueLayer.getGraphics();
            graphics.setColor(Color.BLUE);
            for (int x = 0; x < valueLayer.getWidth(); ++x)
                drawValue(graphics, buffer.peek(x), x);

            drawLayers(false);
        }
    }

    private void drawValue(Graphics2D graphics, float value, int x) {
        final float normalized = 1 - (value - minValue) / (maxValue - minValue);

        final var currentPoint = new Point(x, (int) (normalized * valueLayer.getHeight()));
        if (previousPoint == null)
            graphics.drawRect(currentPoint.x, currentPoint.y, 1, 1);
        else
            graphics.drawLine(previousPoint.x, previousPoint.y, currentPoint.x, currentPoint.y);

        previousPoint = currentPoint;
    }

    public synchronized void append(float value) {
        var old = buffer.push(value);
        if (old == minValue || old == maxValue) {
            maxValue = Float.MIN_VALUE;
            minValue = Float.MAX_VALUE;

            for (float savedValue : buffer.getContents()) {
                maxValue = Math.max(maxValue, savedValue);
                minValue = Math.min(minValue, savedValue);
            }

            fullRedraw();
        } else if (value > maxValue) {
            maxValue = value;
            fullRedraw();
        } else if (value < minValue) {
            minValue = value;
            fullRedraw();
        } else {
            var image = valueLayer.getImage();
            var newImage = new BufferedImage(image.getColorModel(),
                    image.copyData(null),
                    image.isAlphaPremultiplied(),
                    null);
            valueLayer.clear();
            var graphics = valueLayer.getGraphics();
            graphics.drawImage(newImage, -1, 0, null);
            graphics.setColor(Color.BLUE);
            drawValue(graphics, value, valueLayer.getWidth() - 1);
            drawLayers(false);
        }
    }
}
