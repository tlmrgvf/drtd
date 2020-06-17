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


public final class GoertzelFilter extends PipelineComponent<Float, Float> {
    private static BufferedImage IMAGE;
    private static Dimension SIZE = new Dimension();

    static {
        try {
            IMAGE = ImageIO.read(Drtd.readResourceStream("goertzelfilter.png"));
            SIZE = new Dimension(IMAGE.getWidth(), IMAGE.getHeight());
        } catch (IOException e) {
            Drtd.getLogger(BitConverter.class).throwing("GoertzelFilter", "<clinit>", e);
            Utils.die();
        }
    }

    private final RingBuffer sampleBuffer;
    private final float frequency;
    private double coefficient;
    private final int blockSize;

    public GoertzelFilter(int blockSize, float frequency) {
        super(Float.class);
        this.blockSize = blockSize;
        this.sampleBuffer = new RingBuffer(blockSize);
        this.frequency = frequency;
    }

    @Override
    public int onInit(int calculatedInputSampleRate) {
        coefficient = 2 * Math.cos((2 * Math.PI / blockSize) *
                (int) Math.round(frequency * (blockSize / (double) calculatedInputSampleRate)));
        return calculatedInputSampleRate;
    }

    @Override
    protected Float calculate(Float input) {
        sampleBuffer.push(input);

        double value1 = 0;
        double value2 = 0;
        for (int i = 0; i < blockSize; ++i) {
            double value = coefficient * value1 - value2 + sampleBuffer.peek(i);
            value2 = value1;
            value1 = value;
        }

        return (float) Math.sqrt(value2 * value2 + value1 * value1 - coefficient * value1 * value2);
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        return SIZE;
    }

    @Override
    public void drawRelative(Graphics2D graphics) {
        graphics.drawImage(IMAGE, 0, 0, null);
    }
}
