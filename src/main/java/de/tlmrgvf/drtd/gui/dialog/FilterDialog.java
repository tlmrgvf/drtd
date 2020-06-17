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

package de.tlmrgvf.drtd.gui.dialog;

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.dsp.component.firfilter.GenericFirFilter;
import de.tlmrgvf.drtd.dsp.window.Window;
import de.tlmrgvf.drtd.dsp.window.*;
import de.tlmrgvf.drtd.gui.utils.Canvas;
import de.tlmrgvf.drtd.gui.utils.Layer;
import de.tlmrgvf.drtd.utils.Complex;
import de.tlmrgvf.drtd.utils.Constructor;
import de.tlmrgvf.drtd.utils.Utils;
import org.jtransforms.fft.FloatFFT_1D;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.*;
import java.awt.event.*;
import java.awt.font.FontRenderContext;
import java.awt.font.LineMetrics;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.text.ParseException;
import java.util.Arrays;
import java.util.Collections;
import java.util.logging.Logger;

public final class FilterDialog extends JDialog {
    private final static int DB_SCALE_MAX = 200, DB_SCALE_MIN = 4;
    private final static Logger LOGGER = Drtd.getLogger(FilterDialog.class);
    private final static int FFTSIZE = GenericFirFilter.MAX_TAPS * 2;
    private final static int FDIVS = 40;
    private final static int ADIVS = 10;

    private final Layer plot;
    private final GenericFirFilter<?> filter;
    private final Canvas canvas;
    private final JLabel attLabel;
    private final JSlider verticalSize;
    private final JSpinner spinStart;
    private final JSpinner spinEnd;
    private final JSpinner spinAtt;
    private final JSpinner spinTaps;
    private final JCheckBox bandStop;
    private final float[] res = new float[FFTSIZE / 2];

    private float maxMagnitude = 0;
    private float minMagnitude = Float.NaN;
    private WindowType windowType;
    private Window window;

    public FilterDialog(GenericFirFilter<?> filter) {
        super(Drtd.getMainGui().getPipelineDialog());
        this.filter = filter;
        final ListenerImpl listener = new ListenerImpl();

        setTitle("Configure filter");
        setResizable(true);
        setLocation(getParent().getLocation());
        setIconImage(Drtd.ICON);
        setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
        setMinimumSize(new Dimension(400, 400));

        window = filter.getWindow();
        windowType = WindowType.getByInstance(window);
        if (windowType == null) {
            LOGGER.severe("Unknown filter \"" + window + "\"!");
            Utils.die();
        }

        final boolean isKaiser = windowType == WindowType.KAISER;

        canvas = new Canvas();
        canvas.addMouseListener(listener);
        canvas.addComponentListener(listener);
        plot = canvas.createLayer(0, 0, Layer.PARENT_SIZE, Layer.PARENT_SIZE);

        verticalSize = new JSlider(DB_SCALE_MIN, DB_SCALE_MAX, DB_SCALE_MAX);
        verticalSize.setOrientation(SwingConstants.VERTICAL);
        verticalSize.setToolTipText("Vertical size");
        verticalSize.addChangeListener(listener);

        var rootPanel = new JPanel(new BorderLayout()); //Outermost panel
        rootPanel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
        rootPanel.add(verticalSize, BorderLayout.WEST);
        rootPanel.add(canvas, BorderLayout.CENTER);

        var gridLayout = new GridLayout(6, 2);
        gridLayout.setVgap(10);
        gridLayout.setHgap(10);
        JPanel settingsPanel = new JPanel(gridLayout);
        settingsPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),
                "Filter settings"));

        rootPanel.add(settingsPanel, BorderLayout.SOUTH);

        spinTaps = new JSpinner(new SpinnerNumberModel(filter.getTaps(),
                1,
                FFTSIZE / 2,
                2));
        spinTaps.addChangeListener(listener);

        bandStop = new JCheckBox();
        bandStop.addActionListener(listener);
        bandStop.setSelected(filter.isBandStop());

        attLabel = new JLabel("Attenuation:");
        spinAtt = new JSpinner(new SpinnerNumberModel(isKaiser ? ((KaiserWindow) window).getAttenuation() : 20,
                20,
                100,
                1));
        spinAtt.addChangeListener(listener);
        attLabel.setEnabled(isKaiser);
        spinAtt.setEnabled(isKaiser);

        var windowSelector = new JComboBox<>(WindowType.createWindowInstances());
        windowSelector.addItemListener((e) -> {
            Window newWindow = (Window) e.getItem();
            filter.setWindow(newWindow);
            window = newWindow;
            windowType = WindowType.getByInstance(window);
            boolean enable = windowType == WindowType.KAISER;
            attLabel.setEnabled(enable);
            spinAtt.setEnabled(enable);
            plot(true);
        });
        windowSelector.setSelectedItem(filter.getWindow());

        spinStart = new JSpinner(new SpinnerNumberModel(filter.getStartFrequency(),
                0,
                filter.getStopFrequency(),
                1));
        spinStart.addChangeListener(listener);

        spinEnd = new JSpinner(new SpinnerNumberModel(filter.getStopFrequency(),
                filter.getStartFrequency(),
                filter.getSampleRate() / 2,
                1));
        spinEnd.addChangeListener(listener);

        settingsPanel.add(new JLabel("Window:"));
        settingsPanel.add(windowSelector);

        settingsPanel.add(new JLabel("Taps:"));
        settingsPanel.add(spinTaps);

        settingsPanel.add(new JLabel("f start:"));
        settingsPanel.add(spinStart);

        settingsPanel.add(new JLabel("f end:"));
        settingsPanel.add(spinEnd);

        settingsPanel.add(attLabel);
        settingsPanel.add(spinAtt);

        settingsPanel.add(new JLabel("Invert:"));
        settingsPanel.add(bandStop);
        bandStop.setEnabled(filter.getStartFrequency() != filter.getStopFrequency());

        setSize(400, 400);
        add(rootPanel);
        pack();
        plot(true);
    }

    public void updateFromFilter() {
        final boolean isKaiser = windowType == WindowType.KAISER;

        bandStop.setEnabled(filter.getStartFrequency() != filter.getStopFrequency());
        bandStop.setSelected(filter.isBandStop());
        spinTaps.setValue(filter.getTaps());
        spinEnd.setValue(filter.getStopFrequency());
        spinStart.setValue(filter.getStartFrequency());
        spinAtt.setEnabled(isKaiser);

        if (isKaiser)
            spinAtt.setValue(((KaiserWindow) filter.getWindow()).getAttenuation());

        plot(true);
    }

    private void plot(boolean recalculate) {
        final int width = canvas.getWidth(), height = canvas.getHeight();
        final Graphics2D plotGraphics = plot.getGraphics();
        final FontRenderContext frc = plotGraphics.getFontRenderContext();
        final Font font = Utils.FONT.deriveFont(12F);
        plotGraphics.addRenderingHints(Collections.singletonMap(RenderingHints.KEY_ANTIALIASING,
                RenderingHints.VALUE_ANTIALIAS_ON));

        //Make sure we reset all transforms before using graphics
        AffineTransform transform = new AffineTransform();
        transform.setToIdentity();
        plotGraphics.setTransform(transform);

        //Clear canvas
        plotGraphics.setColor(Color.BLACK);
        plotGraphics.fillRect(0, 0, canvas.getWidth(), canvas.getHeight());
        plotGraphics.setColor(Color.WHITE);
        plotGraphics.setFont(font);

        if (recalculate) {
            /* Calculate FFT */
            final FloatFFT_1D fft = new FloatFFT_1D(FFTSIZE);
            final float[] coefficients = filter.getCoefficients();
            final float[] farr = new float[FFTSIZE * 2];
            maxMagnitude = 0;
            minMagnitude = Float.NaN;

            for (int i = 0; i < coefficients.length; ++i) {
                farr[i * 2] = coefficients[i];
            }

            fft.complexForward(farr);

            /* Calculate magnitude and find maximum */
            for (int i = 0; i < res.length; ++i) {
                final float magnitude = new Complex(farr[i * 2], farr[i * 2 + 1]).magnitude();

                res[i] = magnitude;
                maxMagnitude = Math.max(magnitude, maxMagnitude);
                minMagnitude = Float.isNaN(minMagnitude) ? magnitude : Math.min(magnitude, minMagnitude);
            }
        }

        /* Calculate db per division based on user set vertical size */
        final float dbMax = -verticalSize.getValue();
        final float dbPerDiv = -dbMax / (float) ADIVS;

        /* Make information string */
        final String information = String.format(
                "| Max. Att.: %.2fdB | dB/div: %.2f |",
                Utils.calcDbAmplitude(minMagnitude, maxMagnitude),
                dbPerDiv
        );
        final LineMetrics informationMetrics = font.getLineMetrics(information, frc);

        /* Offset of the actual plot in the y direction is the height of the info string */
        final int plotOffsetY = (int) Math.ceil(informationMetrics.getHeight()) + 1;
        int plotHeight = height - plotOffsetY;

        final String zeroDbMarker = "0 dB";

        /* Offset of the actual plot in the x direction, the width of the zero marker */
        final int plotOffsetX = (int) font.getStringBounds(zeroDbMarker, frc).getWidth() + 4;
        int plotWidth = width - plotOffsetX;

        plotGraphics.drawString(zeroDbMarker, 2, (int) (plotOffsetY - informationMetrics.getStrikethroughOffset()));

        /* Translate to actual plot position, so we don't have to constantly add the offset */
        plotGraphics.translate(plotOffsetX, 0);
        plotGraphics.drawString(information, 0, (int) informationMetrics.getAscent());

        plotGraphics.setColor(Color.GRAY);
        plotGraphics.drawLine(0, plotOffsetY - 1, width - 1, plotOffsetY - 1);

        /* Start by drawing the frequency divisions */
        final float frequencyStep = (width - plotOffsetX) / (float) FDIVS;

        int lastX = 0;
        final float halfSampleRate = filter.getSampleRate() / 2F;
        final float fStepPerDiv = halfSampleRate / (FDIVS - 1F);
        for (float x = 0; x < FDIVS; ++x) {
            /*
             * The last line to be labeled will be the second from right, not the one right on the border, that's why we
             * scale to halfSampleRate. Make sure the drawing of the response further below does the same, or else
             * the labeling will be wrong
             */
            final float f = (float) Utils.scaleLog(
                    x * fStepPerDiv,
                    0,
                    halfSampleRate,
                    0,
                    halfSampleRate,
                    true
            );

            String frequency;
            if (f >= 1000) {
                frequency = String.format("%.1fkHz", f / 1000F);
            } else {
                frequency = String.format("%.1fHz", f);
            }

            Rectangle2D bounds = font.getStringBounds(frequency, frc).getBounds2D();
            final double strWidth = bounds.getWidth() + 10;
            final double strHeight = bounds.getHeight();

            /* Check if we can draw the label without interfering with other labels */
            plotHeight = Math.min(plotHeight, height - plotOffsetY - (int) strWidth);
            int xt = (int) (x * frequencyStep - (strHeight / 4F));
            if (xt - lastX < strHeight && x != 0) {
                continue;
            }

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
            int xPos = (int) (x * frequencyStep);
            plotGraphics.drawLine(xPos, plotOffsetY, xPos, plotOffsetY + plotHeight);
        }

        /* Draw vertical dividers */
        float attStep = plotHeight / (float) ADIVS;
        for (int i = 1; i <= ADIVS; ++i) {
            int yPos = (int) (attStep * i) + plotOffsetY;
            plotGraphics.drawLine(0, yPos, width - 1, yPos);
        }

        /* Now plot the values */
        plotGraphics.setColor(Color.WHITE);
        Point last = null;
        final int lowerLine = plotHeight + plotOffsetY;
        final float fStepPerPixel = halfSampleRate / (plotWidth - 1F);
        for (int x = 0; x < plotWidth; ++x) {
            final float logIndex = (float) Utils.scaleLog(
                    x * fStepPerPixel,
                    0,
                    halfSampleRate,
                    0,
                    res.length - 1,
                    true
            );

            double attenuation = Utils.calcDbAmplitude(Utils.linearInterpolate(logIndex, res), maxMagnitude);
            final int y = (int) Math.round((attenuation / dbMax) * plotHeight) + plotOffsetY;

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

        canvas.drawLayers(true);
        Drtd.getMainGui().getPipelineDialog().redraw();
    }

    private enum WindowType {
        RECTANGULAR(RectangularWindow::new, RectangularWindow.class),
        BLACKMAN(BlackmanWindow::new, BlackmanWindow.class),
        HAMMING(HammingWindow::new, HammingWindow.class),
        HANN(HannWindow::new, HannWindow.class),
        KAISER(KaiserWindow::new, KaiserWindow.class);

        private final Constructor<Window> constructor;
        private final Class<? extends Window> classObj;


        WindowType(Constructor<Window> constructor, Class<? extends Window> classObj) {
            this.constructor = constructor;
            this.classObj = classObj;
        }

        public static Window[] createWindowInstances() {
            return Arrays.stream(values()).map(w -> w.constructor.construct()).toArray(Window[]::new);
        }

        public static WindowType getByInstance(Window window) {
            for (WindowType type : values()) {
                if (type.classObj.isInstance(window)) return type;
            }
            return null;
        }
    }

    private class ListenerImpl extends MouseAdapter implements ChangeListener, ComponentListener, ActionListener {

        @Override
        public void componentResized(ComponentEvent e) {
            plot(false);
        }

        @Override
        public void actionPerformed(ActionEvent actionEvent) {
            if (actionEvent.getSource().equals(bandStop))
                filter.setBandStop(bandStop.isSelected());

            plot(true);
        }

        @Override
        public void stateChanged(ChangeEvent changeEvent) {
            Object source = changeEvent.getSource();

            if (source.equals(spinStart)) {
                int value = (int) spinStart.getValue();
                filter.setStartFrequency(value);
                ((SpinnerNumberModel) spinEnd.getModel()).setMinimum(value);
            } else if (source.equals(spinEnd)) {
                int value = (int) spinEnd.getValue();
                filter.setStopFrequency(value);
                ((SpinnerNumberModel) spinStart.getModel()).setMaximum(value);
            } else if (source.equals(spinAtt)) {
                ((KaiserWindow) window).setAttenuation((int) spinAtt.getValue());
                filter.recalculateFilterCoefficients();
            } else if (source.equals(spinTaps)) {
                int value = (int) spinTaps.getValue();
                if (value % 2 == 0) {
                    value--;
                    spinTaps.setValue(value);
                }

                filter.setTaps(value);
            }

            plot(!source.equals(verticalSize));
            bandStop.setEnabled(filter.getStartFrequency() != filter.getStopFrequency());
        }

        @Override
        public void mouseClicked(MouseEvent mouseEvent) {
            try {
                spinAtt.commitEdit();
                spinEnd.commitEdit();
                spinStart.commitEdit();
                spinTaps.commitEdit();
                plot(true);
            } catch (ParseException e) {
                JOptionPane.showMessageDialog(FilterDialog.this,
                        String.format("Invalid entry at position %d!", e.getErrorOffset() + 1), "Format error",
                        JOptionPane.ERROR_MESSAGE);
            }
        }

        @Override
        public void componentMoved(ComponentEvent e) {

        }

        @Override
        public void componentShown(ComponentEvent e) {
        }

        @Override
        public void componentHidden(ComponentEvent e) {

        }
    }
}
