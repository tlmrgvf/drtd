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

import java.awt.*;

public final class Mapper<T, U> extends PipelineComponent<T, U> {
    private final static Dimension SIZE = new Dimension(20, 16);
    private final MapFunction<T, U> function;

    public Mapper(Class<U> resultClass, MapFunction<T, U> function) {
        super(resultClass, false);
        this.function = function;
    }

    @Override
    protected U calculate(T input) {
        return function.map(input);
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        return SIZE;
    }

    @Override
    public void drawRelative(Graphics2D g) {
        g.drawRect(0, 0, SIZE.width - 1, SIZE.height - 1);
        g.translate(3, 3);
        final var width = SIZE.width - 7;
        final var height = SIZE.height - 7;
        final int position = (int) (width * .7F);

        for (int i = 0; i < 3; ++i) {
            g.drawLine(position - i, 0, width - i, height / 2);
            g.drawLine(position - i, height, width - i, height / 2 + 1);
        }

        g.drawLine(0, height / 2, width, height / 2);
        g.drawLine(0, height / 2 + 1, width, height / 2 + 1);
        g.drawLine(0, 0, 0, height);
        g.drawLine(1, 0, 1, height);
    }

    public interface MapFunction<T, U> {
        U map(T in);
    }
}
