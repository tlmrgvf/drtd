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

package de.tlmrgvf.drtd.dsp;

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.dsp.pipeline.PipelineBuilder;
import de.tlmrgvf.drtd.gui.MainGui;
import de.tlmrgvf.drtd.utils.Utils;

import javax.swing.*;
import java.awt.*;
import java.util.Set;
import java.util.logging.Logger;

public abstract class PipelineComponent<T, U> {
    public final static int LINE_WIDTH = 2;
    public final static int COMPONENT_HORIZONTAL_SPACING = 18 + LINE_WIDTH;
    public final static int COMPONENT_VERTICAL_SPACING = 10;
    private final static int MARKER_MARGIN = 2 + LINE_WIDTH;
    private final static Logger LOGGER = Drtd.getLogger(PipelineComponent.class);
    private final static MainGui MAIN_GUI = Drtd.getMainGui();

    private final static Point[] MARKER_MONITOR_TARGET = new Point[]{
            new Point(-12, -4),
            new Point(-4, -4),
            new Point(0, 0),
            new Point(0, -4),
            new Point(0, 0),
            new Point(-4, 0)
    };

    private final static Point[] MARKER_MONITOR_SOURCE = new Point[]{
            new Point(0, -4),
            new Point(0, 0),
            new Point(4, 0),
            new Point(0, 0),
            new Point(4, -4),
            new Point(12, -4)
    };

    private static volatile PipelineComponent<?, ?> monitoring;
    private static volatile boolean monitoringOutput;

    private final PipelineComponent<?, ?>[] containing;
    private final Class<U> resultClass;
    private final ComponentType type;
    private final boolean drawAntialiasOn;

    private Point absolutePosition;
    protected Polygon monitorIcon;
    private int inputSampleRate;
    private int outputSampleRate;

    public PipelineComponent(Class<U> resultClass) {
        this(resultClass, true);
    }

    public PipelineComponent(Class<U> resultClass, boolean drawAntialiasOn) {
        this.resultClass = resultClass;
        this.drawAntialiasOn = drawAntialiasOn;
        containing = null;
        type = ComponentType.NORMAL;
    }

    public PipelineComponent(Class<U> resultClass, PipelineComponent<?, ?>[] all, ComponentType type) {
        assert all.length > 0;
        this.type = type;
        this.containing = all;
        this.resultClass = resultClass;
        this.drawAntialiasOn = true;
    }

    public static boolean isMonitoringOutput() {
        return monitoringOutput;
    }

    protected final void drawConnectingLine(Graphics graphics, int fromX, int fromY, int toX, int toY) {
        Graphics2D g = (Graphics2D) graphics.create();
        g.setStroke(new BasicStroke(2F, BasicStroke.CAP_SQUARE, BasicStroke.JOIN_BEVEL, 0F));
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
        g.drawLine(fromX, fromY, toX, toY);
    }

    protected final void drawSimpleConnector(MarkerPosition position, Graphics graphics, Point from, Point to) {
        drawConnectingLine(graphics, from.x + LINE_WIDTH - 1, from.y, to.x - LINE_WIDTH + 1, to.y);

        Graphics g = graphics.create();
        g.setColor(Color.GREEN.darker());
        if (position == MarkerPosition.MARK_SOURCE) {
            g.translate(from.x + MARKER_MARGIN, from.y - MARKER_MARGIN);
            Utils.drawPath(g, MARKER_MONITOR_SOURCE);
        } else if (position == MarkerPosition.MARK_TARGET) {
            g.translate(to.x - MARKER_MARGIN, to.y - MARKER_MARGIN);
            Utils.drawPath(g, MARKER_MONITOR_TARGET);
        }
    }

    public final boolean isBeingMonitored() {
        if (type == ComponentType.PIPELINE)
            return monitoringOutput ? containing[containing.length - 1].isBeingMonitored() :
                    containing[0].isBeingMonitored();

        return monitoring == this;
    }

    public final void setMonitoring(boolean monitoringOutput) {
        synchronized (PipelineComponent.class) {
            LOGGER.finer("Set monitoring to " + this + ", monitoring output: " + monitoringOutput);
            MAIN_GUI.resetInterpreter();
            monitoring = this;
            PipelineComponent.monitoringOutput = monitoringOutput;
        }
    }

    public final ComponentType getType() {
        return type;
    }

    public final Class<U> getResultClass() {
        return resultClass;
    }

    public final void ensureUnique(Set<PipelineComponent<?, ?>> allComponents) {
        if (allComponents.contains(this))
            throw new IllegalStateException("Component " + this + " was added more than once to a pipeline!");

        allComponents.add(this);
        if (containing != null) {
            for (var component : containing)
                component.ensureUnique(allComponents);
        }
    }

    @SuppressWarnings("unchecked")
    public final U calculateGeneric(Object object) {
        synchronized (PipelineComponent.class) {
            if (monitoring == this && !monitoringOutput)
                MAIN_GUI.updateMonitor(object);
        }

        U result = calculate((T) object);

        synchronized (PipelineComponent.class) {
            if (monitoring == this && monitoringOutput)
                MAIN_GUI.updateMonitor(result);
        }

        return result;
    }

    protected abstract U calculate(T input);

    public abstract Dimension calculateSize(Graphics2D g);

    protected abstract void drawRelative(Graphics2D graphics);

    public final void draw(Point absolutePosition, Graphics2D graphics) {
        this.absolutePosition = absolutePosition.getLocation();
        var size = calculateSize(graphics);
        graphics = (Graphics2D) (type == ComponentType.NORMAL ?
                graphics.create(this.absolutePosition.x, this.absolutePosition.y, size.width, size.height) :
                graphics.create());
        graphics.setColor(Color.BLACK);
        graphics.setRenderingHint(RenderingHints.KEY_ANTIALIASING, drawAntialiasOn ? RenderingHints.VALUE_ANTIALIAS_ON :
                RenderingHints.VALUE_ANTIALIAS_OFF);
        drawRelative(graphics);
    }

    public final int getOutputSampleRate() {
        return outputSampleRate;
    }

    public final int getInputSampleRate() {
        return inputSampleRate;
    }

    public final Point getAbsolutePosition() {
        return absolutePosition.getLocation();
    }

    public final PipelineComponent<?, ?> getClickedComponent(Point clickPoint, Graphics2D graphics) {
        var componentSize = new Rectangle(absolutePosition, calculateSize(graphics));

        if (componentSize.contains(clickPoint)) {
            if (containing == null) {
                return this;
            } else {
                for (var comp : containing) {
                    var clicked = comp.getClickedComponent(clickPoint, graphics);
                    if (clicked != null)
                        return clicked;
                }

                if (type == ComponentType.PARALLEL_PIPELINE && monitorIcon.contains(clickPoint))
                    return this;
            }
        }
        return null;
    }

    public final PipelineBuilder<T, U> pipeline() {
        return PipelineBuilder.createForComponent(this);
    }

    public final int init(int calculatedInputSampleRate) {
        inputSampleRate = calculatedInputSampleRate;
        outputSampleRate = onInit(calculatedInputSampleRate);
        LOGGER.finer(this + " input rate " + calculatedInputSampleRate + " output rate " + outputSampleRate);
        return outputSampleRate;
    }

    protected int onInit(int calculatedInputSampleRate) {
        return calculatedInputSampleRate;
    }

    public void showConfigureDialog() {
        JOptionPane.showMessageDialog(
                MAIN_GUI.getPipelineDialog(),
                "Nothing to configure for:\n\"" + getClass().getName() + "\"",
                "Information",
                JOptionPane.INFORMATION_MESSAGE
        );
    }

    public enum MarkerPosition {
        NO_MARKER,
        MARK_SOURCE,
        MARK_TARGET
    }

    public enum ComponentType {
        NORMAL,
        PIPELINE,
        PARALLEL_PIPELINE
    }
}
