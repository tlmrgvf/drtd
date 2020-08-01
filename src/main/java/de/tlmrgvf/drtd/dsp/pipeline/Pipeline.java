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

package de.tlmrgvf.drtd.dsp.pipeline;

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.dsp.PipelineComponent;

import java.awt.*;

public final class Pipeline<T, U> extends PipelineComponent<T, U> {
    private final PipelineComponent<?, ?>[] pipelineComponents;

    Pipeline(Class<U> resultClass, PipelineComponent<?, ?>[] pipelineComponents) {
        super(resultClass, pipelineComponents, ComponentType.PIPELINE);
        assert pipelineComponents.length > 0;
        this.pipelineComponents = pipelineComponents;
    }

    @Override
    public int onInit(int calculatedInputSampleRate) {
        int currentRate = calculatedInputSampleRate;

        for (PipelineComponent<?, ?> comp : pipelineComponents)
            currentRate = comp.init(currentRate);

        return currentRate;
    }

    @Override
    @SuppressWarnings("unchecked")
    protected U calculate(T input) {
        Object intermediate = input;
        for (PipelineComponent<?, ?> comp : pipelineComponents)
            if ((intermediate = comp.calculateGeneric(intermediate)) == null)
                return null;

        return (U) intermediate;
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        int maxHeight = 0;
        int widthSum = 0;
        for (PipelineComponent<?, ?> comp : pipelineComponents) {
            Dimension size = comp.calculateSize(g);
            maxHeight = Math.max(size.height, maxHeight);
            widthSum += size.width + COMPONENT_HORIZONTAL_SPACING;
        }

        widthSum -= COMPONENT_HORIZONTAL_SPACING;
        return new Dimension(widthSum, maxHeight);
    }

    @Override
    public void drawRelative(Graphics2D graphics) {
        Point absolutePosition = getAbsolutePosition();
        int center = calculateSize((Graphics2D) graphics.create()).height / 2;
        absolutePosition.translate(0, center);

        for (int i = 0; i < pipelineComponents.length; ++i) {
            PipelineComponent<?, ?> comp = pipelineComponents[i];
            Dimension size = comp.calculateSize((Graphics2D) graphics.create());
            comp.draw(new Point(absolutePosition.x, absolutePosition.y - size.height / 2), graphics);
            absolutePosition.translate(size.width + COMPONENT_HORIZONTAL_SPACING, 0);

            if (i < pipelineComponents.length - 1) {
                MarkerPosition position = MarkerPosition.NO_MARKER;

                if (comp.isBeingMonitored() && PipelineComponent.isMonitoringOutput())
                    position = MarkerPosition.MARK_SOURCE;
                else if (pipelineComponents[i + 1].isBeingMonitored() && !PipelineComponent.isMonitoringOutput())
                    position = MarkerPosition.MARK_TARGET;

                graphics.setColor(Drtd.getInterpreter(comp.getResultClass()).getColor());
                drawSimpleConnector(position,
                        graphics,
                        new Point(absolutePosition.x - COMPONENT_HORIZONTAL_SPACING, absolutePosition.y),
                        absolutePosition);
            }
        }
    }
}
