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

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.utils.Utils;
import de.tlmrgvf.drtd.utils.structure.RingBuffer;

import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.IOException;

public final class Normalizer extends PipelineComponent<Float, Float> {
    private static BufferedImage IMAGE;
    private static Dimension SIZE = new Dimension();

    static {
        try {
            IMAGE = ImageIO.read(Drtd.readResourceStream("normalizer.png"));
            SIZE = new Dimension(IMAGE.getWidth(), IMAGE.getHeight());
        } catch (IOException e) {
            Drtd.getLogger(BitConverter.class).throwing("Normalizer", "<clinit>", e);
            Utils.die();
        }
    }

    private final RingBuffer delayBuffer;
    private int windowSize;
    private float max = Float.MIN_VALUE;
    private float min = Float.MAX_VALUE;
    private float factor = 1;
    private float offset = 0;
    private int count;

    public Normalizer(int windowSize) {
        super(Float.class);
        delayBuffer = new RingBuffer(windowSize);
        this.windowSize = windowSize;
    }

    public int getWindowSize() {
        return windowSize;
    }

    public synchronized void setWindowSize(int windowSize) {
        this.windowSize = windowSize;
        delayBuffer.resize(windowSize);
        delayBuffer.clear(true);
        factor = 1;
        count = 0;
        max = Float.MIN_VALUE;
        min = Float.MAX_VALUE;
    }

    @Override
    protected synchronized Float calculate(Float input) {
        final float ret = (delayBuffer.push(input) - offset) * factor;

        if (count >= windowSize) {
            count = 0;
            offset = min;
            factor = 1 / (max - min);
            max = Float.MIN_VALUE;
            min = Float.MAX_VALUE;
        } else {
            ++count;
            max = Math.max(max, input);
            min = Math.min(min, input);
        }

        return Float.isNaN(ret) ? 0 : ret;
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        return SIZE;
    }

    @Override
    protected void drawRelative(Graphics2D graphics) {
        graphics.drawImage(IMAGE, 0, 0, null);
    }
}
