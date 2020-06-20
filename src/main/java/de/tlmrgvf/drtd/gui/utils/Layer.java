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

import java.awt.*;
import java.awt.image.BufferedImage;

public final class Layer {
    public final static int PARENT_SIZE = -1;

    private BufferedImage image;
    private BufferedImage snapshot;
    private Graphics2D graphics;
    private final Canvas canvas;
    private int x;
    private int y;
    private int width;
    private int height;
    private boolean parentWidth;
    private boolean parentHeight;
    private boolean buffer;

    public Layer(Canvas canvas, int x, int y, int width, int height) {
        this.canvas = canvas;
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        this.parentWidth = width == PARENT_SIZE;
        this.parentHeight = height == PARENT_SIZE;
        image = new BufferedImage(getWidth(), getHeight(), BufferedImage.TYPE_INT_ARGB);
        graphics = image.createGraphics();
    }

    public void resize(int width, int height) {
        if (width <= 0 || height <= 0) return;
        graphics.dispose();
        if (parentWidth) this.width = width;
        if (parentHeight) this.height = height;

        if (getWidth() <= 0 || getHeight() <= 0) return;
        BufferedImage bufferedImage = new BufferedImage(getWidth(), getHeight(), BufferedImage.TYPE_INT_ARGB);
        graphics = bufferedImage.createGraphics();

        graphics.drawImage(image, 0, 0, null);
        image = bufferedImage;
    }

    public void clear() {
        if (image.getWidth() != getWidth() || image.getHeight() != getHeight())
            resize(getWidth(), getHeight());

        graphics.dispose();
        graphics = image.createGraphics(); //Remove transforms etc.
        graphics.setComposite(AlphaComposite.Clear);
        graphics.fillRect(0, 0, getWidth(), getHeight());
        graphics.setComposite(AlphaComposite.SrcOver);
        graphics.setColor(Color.BLACK);
    }

    public void createSnapshot() {
        snapshot = new BufferedImage(image.getColorModel(),
                image.copyData(null),
                image.isAlphaPremultiplied(),
                null);
    }

    public void drawOn(Graphics toDrawOn) {
        if (getWidth() == 0 || getHeight() == 0) return;
        toDrawOn.drawImage(snapshot, x, y, null);
    }

    public int getWidth() {
        return parentWidth ? canvas.getWidth() : width;
    }

    public void setWidth(int width) {
        parentWidth = width == PARENT_SIZE;
        this.width = parentWidth ? canvas.getWidth() - canvas.getInsets().left - canvas.getInsets().right : width;
    }

    public int getHeight() {
        return parentHeight ? canvas.getHeight() - canvas.getInsets().top - canvas.getInsets().bottom : height;
    }

    public void setHeight(int height) {
        parentHeight = height == PARENT_SIZE;
        this.height = parentHeight ? canvas.getHeight() : height;
    }

    public BufferedImage getImage() {
        return image;
    }

    public int getX() {
        return x;
    }

    public void setX(int x) {
        this.x = x;
    }

    public int getY() {
        return y;
    }

    public void setY(int y) {
        this.y = y;
    }

    public Graphics2D getGraphics() {
        return graphics;
    }
}
