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

package de.tlmrgvf.drtd.gui.utils;

import javax.swing.*;
import javax.swing.plaf.ComponentUI;
import java.awt.*;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.util.ArrayList;
import java.util.List;

public class Canvas extends JComponent {
    private final static int UPDATE_LIMIT = 30;
    private final List<Layer> layers = new ArrayList<>();
    private long lastUpdate;
    private CanvasResizeListener resizeListener;

    public Canvas() {
        super();
        setDoubleBuffered(true);
        setUI(new ComponentUI() {
            @Override
            public void paint(Graphics g, JComponent c) {
                super.paint(g, c);

                g.setColor(getBackground());
                g.fillRect(0, 0, getWidth(), getHeight());
                layers.forEach(layer -> layer.drawOn(g));
            }
        });
        addComponentListener(new ComponentAdapter() {
            @Override
            public void componentResized(ComponentEvent e) {
                resizeLayers();
                if (resizeListener != null)
                    resizeListener.onCanvasResized(Canvas.this);
            }
        });
    }

    public void setResizeListener(CanvasResizeListener resizeListener) {
        this.resizeListener = resizeListener;
    }

    @Override
    public int getWidth() {
        return Math.max(1, super.getWidth());
    }

    @Override
    public int getHeight() {
        return Math.max(1, super.getHeight());
    }

    public boolean willDraw() {
        return (System.currentTimeMillis() - lastUpdate) > (1 / (float) UPDATE_LIMIT) * 1000;
    }

    public void drawLayers(boolean force) {
        if (!isVisible()) return;

        long time = System.currentTimeMillis();
        if (time - lastUpdate < (1 / (float) UPDATE_LIMIT) * 1000 && !force)
            return;
        lastUpdate = time;

        layers.forEach(Layer::createSnapshot);
        super.repaint();
    }

    public void resizeLayers() {
        layers.forEach(layer -> layer.resize(getWidth(), getHeight()));
    }

    public Layer createLayer(int x, int y, int width, int height) {
        var lay = new Layer(this, x, y, width, height);
        layers.add(lay);
        return lay;
    }

    public interface CanvasResizeListener {
        void onCanvasResized(Canvas c);
    }
}
