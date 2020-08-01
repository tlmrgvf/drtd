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
import de.tlmrgvf.drtd.utils.Complex;

import java.awt.*;

public final class AngleDifference extends PipelineComponent<Complex, Float> {
    private final static Dimension SIZE = new Dimension(35, 30);
    private float previousAngle;

    public AngleDifference() {
        super(Float.class);
    }

    @Override
    protected Float calculate(Complex sample) {
        final float angle = sample.angle();
        final float diff = angle - previousAngle;
        int sign = (int) Math.signum(diff);
        float absDiff = Math.abs(diff);
        previousAngle = angle;

        /*
         * If the angle difference is bigger than 180 degrees, we are either getting a signal with a frequency of more
         * than half the sample frequency, or we wrapped around. Let's ignore the first case, and just assume
         * that our input frequency will always be in a valid range 0 - (samplerate / 2)
         */
        if (absDiff > Math.PI) {
            absDiff = (float) (2 * Math.PI - absDiff);
            sign *= -1;
        }

        return sign * absDiff;
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        return SIZE;
    }

    @Override
    public void drawRelative(Graphics2D g) {
        g.drawRect(0, 0, SIZE.width - 1, SIZE.height - 1);
        g.translate(5, 5);
        final int width = SIZE.width - 11;
        final int height = SIZE.height - 11;

        final int centerX = (int) Math.sqrt((width * width * .49) / 2);
        final int centerY = (int) Math.sqrt((height * height * .49) / 2);

        g.drawLine(0, height, width, 0);
        g.drawLine(0, height, width, height);

        g.drawLine(centerX, height - centerY, (int) (.7 * width), height);

        Polygon triangle = new Polygon(new int[]{0, 2, 4}, new int[]{0, -3, 0}, 3);
        triangle.translate(8, height - 2);
        g.drawPolygon(triangle);
    }
}
