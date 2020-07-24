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

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.gui.utils.Canvas;
import de.tlmrgvf.drtd.gui.utils.Layer;
import de.tlmrgvf.drtd.utils.Utils;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.util.Collections;

public final class FilterPlot extends JPanel {
    private final static int DB_SCALE_MAX = 200, DB_SCALE_MIN = 4;
    private final static int FDIVS = 40;
    private final static int ADIVS = 10;

    private final Layer plot;
    private final Layer marker;
    private final Canvas canvas;
    private final JSlider verticalSize;
    private final int halfSampleRate;

    private float[] values;
    private float min;
    private float max;
    private boolean showMarker = false;
    private boolean linear = false;
    private int markerX = 0;

    public FilterPlot(int sampleRate) {
        halfSampleRate = sampleRate / 2;
        final var listener = new ListenerImpl();
        setLayout(new BorderLayout());

        canvas = new Canvas();
        canvas.addMouseListener(listener);
        canvas.addMouseMotionListener(listener);
        canvas.setToolTipText("Middle mouse click to switch between log/linear scale");
        canvas.setResizeListener((c) -> plot(true));
        plot = canvas.createLayer(0, 0, Layer.PARENT_SIZE, Layer.PARENT_SIZE);
        marker = canvas.createLayer(0, 0, 1, 1);

        verticalSize = new JSlider(DB_SCALE_MIN, DB_SCALE_MAX, DB_SCALE_MAX);
        verticalSize.setOrientation(SwingConstants.VERTICAL);
        verticalSize.setToolTipText("dB/Div");
        verticalSize.addChangeListener(listener);

        add(verticalSize, BorderLayout.WEST);
        add(canvas, BorderLayout.CENTER);
    }

    public void plot(float[] values, float min, float max) {
        this.values = values;
        this.min = min;
        this.max = max;
        plot(true);
        Drtd.getMainGui().getPipelineDialog().redraw();
    }

    public void plot(boolean forceDraw) {
        final int width = canvas.getWidth();
        final int height = canvas.getHeight();
        final var plotGraphics = plot.getGraphics();
        final var frc = plotGraphics.getFontRenderContext();
        final var font = Utils.FONT.deriveFont(12F);
        plotGraphics.addRenderingHints(Collections.singletonMap(RenderingHints.KEY_ANTIALIASING,
                RenderingHints.VALUE_ANTIALIAS_ON));

        /* Make sure we reset all transforms before using graphics */
        var transform = new AffineTransform();
        transform.setToIdentity();
        plotGraphics.setTransform(transform);

        /* Clear canvas */
        plotGraphics.setColor(Color.BLACK);
        plotGraphics.fillRect(0, 0, canvas.getWidth(), canvas.getHeight());
        plotGraphics.setColor(Color.WHITE);
        plotGraphics.setFont(font);

        /* Calculate db per division based on user set vertical size */
        final float dbMax = -verticalSize.getValue();
        final float dbPerDiv = -dbMax / (float) ADIVS;

        final var informationMetrics = font.getLineMetrics("|", frc);

        /* Offset of the actual plot in the y direction is the height of the info string */
        final int plotOffsetY = (int) Math.ceil(informationMetrics.getHeight()) + 1;
        int plotHeight = height - plotOffsetY;

        final var upperDbMarker = String.format("%.2fdB", Utils.calcDbAmplitude(max, 1));

        /* Offset of the actual plot in the x direction, the width of the zero marker */
        final int plotOffsetX = (int) font.getStringBounds(upperDbMarker, frc).getWidth() + 4;
        final int plotWidth = width - plotOffsetX - 15;

        plotGraphics.drawString(upperDbMarker, 2, (int) (plotOffsetY - informationMetrics.getStrikethroughOffset()));

        /* Translate to actual plot position, so we don't have to constantly add the offset */
        plotGraphics.translate(plotOffsetX, 0);

        plotGraphics.setColor(Color.GRAY);
        plotGraphics.drawLine(0, plotOffsetY - 1, plotWidth, plotOffsetY - 1);

        /* Start by drawing the frequency divisions */
        final float markerStep = plotWidth / (float) FDIVS;
        int lastX = 0;
        for (float x = 0; x <= FDIVS; ++x) {
            final float f = (float) (linear ? x * markerStep * halfSampleRate / (float) plotWidth : Utils.scaleLog(
                    x,
                    0,
                    FDIVS,
                    0,
                    halfSampleRate,
                    false
            ));

            String frequency;
            if (f >= 1000)
                frequency = String.format("%.2f kHz", f / 1000F);
            else
                frequency = String.format("%.2f Hz", f);

            Rectangle2D bounds = font.getStringBounds(frequency, frc).getBounds2D();
            final double strWidth = bounds.getWidth() + 10;
            final double strHeight = bounds.getHeight();

            /* Check if we can draw the label without interfering with other labels */
            plotHeight = Math.min(plotHeight, height - plotOffsetY - (int) strWidth);
            int xt = (int) (x * markerStep - (strHeight / 4F));
            if (xt - lastX < strHeight && x != 0)
                continue;

            /* Draw label */
            int yt = (int) (height - strWidth) + 5;
            plotGraphics.translate(xt, yt);
            plotGraphics.rotate(Math.toRadians(90));
            plotGraphics.drawString(frequency, 0, 0);
            plotGraphics.rotate(Math.toRadians(-90));
            plotGraphics.translate(-xt, -yt);
            lastX = xt;
        }

        /* Draw horizontal dividers */
        for (int x = 0; x <= FDIVS; ++x) {
            int xPos = (int) (x * markerStep);
            plotGraphics.drawLine(xPos, plotOffsetY, xPos, plotOffsetY + plotHeight);
        }

        /* Draw vertical dividers */
        float attStep = plotHeight / (float) ADIVS;
        for (int i = 1; i <= ADIVS; ++i) {
            int yPos = (int) (attStep * i) + plotOffsetY;
            plotGraphics.drawLine(0, yPos, plotWidth - 1, yPos);
        }

        final int markX = Utils.clampBetween(0, plotWidth, markerX - plotOffsetX);
        float markerAtt = 0;

        /* Now plot the values */
        plotGraphics.setColor(Color.WHITE);
        Point last = null;
        final int lowerLine = plotHeight + plotOffsetY;
        final float maxAtt = (float) Utils.calcDbAmplitude(max, 1);
        for (int x = 0; x < plotWidth; ++x) {
            final float logIndex = (float) (linear ? x * (values.length - 1) / (float) plotWidth : Utils.scaleLog(
                    x,
                    0,
                    plotWidth,
                    0,
                    values.length - 1,
                    false
            ));

            float attenuation = (float) Utils.calcDbAmplitude(Utils.linearInterpolate(logIndex, values), 1);
            if (x == markX)
                markerAtt = attenuation;
            attenuation -= maxAtt;
            final int y = Math.round(attenuation / dbMax * plotHeight) + plotOffsetY;

            /* Connect last point to next point */
            if (last == null) last = new Point(x, y);
            if (y > lowerLine && last.y > lowerLine) {
                last.x = x;
                continue;
            }

            plotGraphics.drawLine(last.x, Math.min(last.y, lowerLine), x, Math.min(y, lowerLine));
            last.x = x;
            last.y = y;
        }

        plotGraphics.setColor(Color.WHITE);

        final float markerF = (float) (float) (linear ? markX * halfSampleRate / (float) plotWidth : Utils.scaleLog(
                markX,
                0,
                plotWidth,
                0,
                halfSampleRate,
                false));

        final var markerString = String.format("Mark freq.: %.2f %s | Mark att.: %.2fdB",
                markerF > 1000 ? markerF / 1000F : markerF,
                markerF > 1000 ? "kHz" : "Hz",
                markerAtt
        );

        /* Make information string */
        final var information = String.format(
                "| Max. Att.: %.2fdB | dB/div: %.2f | %s",
                Utils.calcDbAmplitude(min, 1),
                dbPerDiv,
                showMarker ? markerString : ""
        );
        plotGraphics.drawString(information, 0, (int) informationMetrics.getAscent());

        if (showMarker) {
            marker.setX(markX + plotOffsetX);
            marker.setY(plotOffsetY);
            if (plotHeight != marker.getHeight())
                marker.setHeight(plotHeight);

            Graphics2D graphics = marker.getGraphics();
            graphics.setStroke(new BasicStroke(1,
                    BasicStroke.CAP_SQUARE,
                    BasicStroke.JOIN_ROUND,
                    0,
                    new float[]{5},
                    0));
            graphics.setColor(Color.YELLOW);
            graphics.drawLine(0, 0, 0, plotHeight);
        } else {
            marker.clear();
        }

        canvas.drawLayers(forceDraw);
    }

    private class ListenerImpl extends MouseAdapter implements ChangeListener {
        @Override
        public void stateChanged(ChangeEvent changeEvent) {
            plot(false);
        }

        @Override
        public void mouseDragged(MouseEvent mouseEvent) {
            if ((mouseEvent.getModifiersEx() & MouseEvent.BUTTON1_DOWN_MASK) != 0) {
                showMarker = true;
                markerX = mouseEvent.getX();
                plot(false);
            }
        }

        @Override
        public void mouseClicked(MouseEvent mouseEvent) {
            int mod = mouseEvent.getButton();
            if (mod == MouseEvent.BUTTON1) {
                showMarker = mouseEvent.getClickCount() == 1;
                markerX = mouseEvent.getX();
                plot(true);
            } else if (mod == MouseEvent.BUTTON2) {
                linear = !linear;
                plot(true);
            }
        }
    }
}
