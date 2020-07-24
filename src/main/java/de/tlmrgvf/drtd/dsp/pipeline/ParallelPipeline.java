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
import java.lang.reflect.Array;
import java.util.Arrays;
import java.util.List;

public final class ParallelPipeline<T, U, V> extends PipelineComponent<T, V> {
    private final static int MARKER_ICON_COUNT = 3;
    private final static int[][] MARKER_ICON_LOCATIONS = new int[][]{
            {-6, -6, 6}, //x
            {6, -6, 0} //y
    };

    private final static Rectangle MARKER_SIZE =
            new Polygon(MARKER_ICON_LOCATIONS[0], MARKER_ICON_LOCATIONS[1], MARKER_ICON_COUNT).getBounds();
    private final static int PIPELINE_HORIZONTAL_SPACING = 14 + LINE_WIDTH;
    private final static int PIPELINE_HORIZONTAL_SPACING_ADJUSTED =
            PIPELINE_HORIZONTAL_SPACING - (MARKER_SIZE.width / 2);

    private final MergeFunction<U, V> func;
    private final List<PipelineComponent<T, U>> pipelineComponents;
    private final U[] us;
    private final Color inputColor;
    private final Color intermediateColor;

    @SuppressWarnings("unchecked")
    ParallelPipeline(Class<?> inputClass,
                     Class<V> resultClass,
                     MergeFunction<U, V> func,
                     List<PipelineComponent<T, U>> pipelineComponents) {
        super(resultClass, pipelineComponents.toArray(PipelineComponent[]::new), ComponentType.PARALLEL_PIPELINE);
        assert pipelineComponents.size() > 1;

        if (Drtd.isGuiMode()) {
            this.inputColor = Drtd.getInterpreter(inputClass).getColor();
            this.intermediateColor = Drtd.getInterpreter(pipelineComponents.get(0).getResultClass()).getColor();
        } else {
            this.inputColor = null;
            this.intermediateColor = null;
        }

        this.func = func;
        this.pipelineComponents = pipelineComponents;
        us = (U[]) Array.newInstance(pipelineComponents.get(0).getResultClass(), pipelineComponents.size());
    }

    @Override
    public int onInit(int calculatedInputSampleRate) {
        int outputSampleRate = -1;
        for (var comp : pipelineComponents) {
            int sr = comp.init(calculatedInputSampleRate);
            if (outputSampleRate < 0)
                outputSampleRate = sr;
            else if (sr != outputSampleRate)
                throw new IllegalStateException("Resulting sample rates of components differ!");
        }

        return outputSampleRate;
    }

    @Override
    protected V calculate(T input) {
        int usIndex = 0;
        boolean abort = false;
        for (var comp : pipelineComponents)
            if ((us[usIndex++] = comp.calculateGeneric(input)) == null)
                abort = true;

        return abort ? null : func.merge(us);
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        int heightSum = 0;
        int maxWidth = 0;
        for (var comp : pipelineComponents) {
            var size = comp.calculateSize(g);
            maxWidth = Math.max(size.width, maxWidth);
            heightSum += size.height + COMPONENT_VERTICAL_SPACING;
        }

        heightSum -= COMPONENT_VERTICAL_SPACING;

        return new Dimension(maxWidth + PIPELINE_HORIZONTAL_SPACING + PIPELINE_HORIZONTAL_SPACING_ADJUSTED,
                heightSum);
    }

    @Override
    protected void drawRelative(Graphics2D graphics) {
        var absolutePosition = getAbsolutePosition();
        absolutePosition.translate(PIPELINE_HORIZONTAL_SPACING_ADJUSTED, 0);

        int markerIndex = -1;
        Point[] leftPoints = new Point[pipelineComponents.size()];
        Point[] rightPoints = new Point[pipelineComponents.size()];

        int pointIndex = 0;
        for (var comp : pipelineComponents) {
            if (comp.isBeingMonitored())
                markerIndex = pointIndex;

            var size = comp.calculateSize((Graphics2D) graphics.create());
            comp.draw(absolutePosition, graphics);

            leftPoints[pointIndex] = new Point(absolutePosition.x - PIPELINE_HORIZONTAL_SPACING_ADJUSTED,
                    absolutePosition.y + size.height / 2);
            rightPoints[pointIndex] = new Point(absolutePosition.x + size.width,
                    absolutePosition.y + size.height / 2);

            absolutePosition.translate(0, size.height + COMPONENT_VERTICAL_SPACING);
            ++pointIndex;
        }

        graphics.setColor(inputColor);
        drawOpeningConnectors(graphics, leftPoints, markerIndex);
        drawClosingConnectors(graphics, rightPoints, intermediateColor, markerIndex);
    }

    private void drawOpeningConnectors(Graphics g, Point[] sourcePoints, int markerIndex) {
        g = g.create();
        int xTarget;
        MarkerPosition position = MarkerPosition.NO_MARKER;

        if (!isMonitoringOutput())
            position = MarkerPosition.MARK_TARGET;

        xTarget = sourcePoints[0].x + PIPELINE_HORIZONTAL_SPACING_ADJUSTED;

        Point previous = null;
        for (var src : sourcePoints) {
            if (previous == null)
                previous = src;
            else
                drawConnectingLine(g, sourcePoints[0].x, previous.y, sourcePoints[0].x, src.y);
        }

        int index = 0;
        for (var src : sourcePoints) {
            drawSimpleConnector(index == markerIndex ? position : MarkerPosition.NO_MARKER,
                    g, new Point(src.x, src.y),
                    new Point(xTarget, src.y));
            ++index;
        }
    }

    private void drawClosingConnectors(Graphics2D g,
                                       Point[] sourcePoints,
                                       Color intermediate,
                                       int markerIndex) {
        var size = calculateSize((Graphics2D) g.create());
        int xTarget;
        g = (Graphics2D) g.create();
        int minY = Integer.MAX_VALUE;
        int maxY = Integer.MIN_VALUE;
        monitorIcon = new Polygon(MARKER_ICON_LOCATIONS[0], MARKER_ICON_LOCATIONS[1], MARKER_ICON_COUNT);
        MarkerPosition position = MarkerPosition.NO_MARKER;

        if (isMonitoringOutput())
            position = MarkerPosition.MARK_SOURCE;

        xTarget = (int) (Arrays.stream(sourcePoints)
                .map(Point::getX)
                .max(Double::compareTo)
                .orElse(0D) + PIPELINE_HORIZONTAL_SPACING_ADJUSTED);

        g.setColor(intermediate);
        for (var src : sourcePoints) {
            minY = Math.min(minY, src.y);
            maxY = Math.max(maxY, src.y);
        }

        drawConnectingLine(g, xTarget, minY, xTarget, maxY);

        int index = 0;
        for (var src : sourcePoints) {
            drawSimpleConnector(index == markerIndex ? position : MarkerPosition.NO_MARKER,
                    g, new Point(src.x, src.y),
                    new Point(xTarget, src.y));
            ++index;
        }

        g.setColor(Color.BLACK);
        monitorIcon.translate(xTarget + 1, getAbsolutePosition().y + size.height / 2);
        g.fillPolygon(monitorIcon);
    }
}