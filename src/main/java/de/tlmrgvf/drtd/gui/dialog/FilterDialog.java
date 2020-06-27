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
import de.tlmrgvf.drtd.gui.component.FilterPlot;
import de.tlmrgvf.drtd.utils.Complex;
import de.tlmrgvf.drtd.utils.Provider;
import de.tlmrgvf.drtd.utils.Utils;
import org.jtransforms.fft.FloatFFT_1D;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.*;
import java.awt.event.*;
import java.text.ParseException;
import java.util.Arrays;
import java.util.logging.Logger;

public final class FilterDialog extends JDialog {
    private final static Logger LOGGER = Drtd.getLogger(FilterDialog.class);
    private final static int FFTSIZE = GenericFirFilter.MAX_TAPS * 2;

    private final FilterPlot plot;
    private final GenericFirFilter<?> filter;
    private final JLabel attLabel;
    private final JSpinner spinStart;
    private final JSpinner spinEnd;
    private final JSpinner spinAtt;
    private final JSpinner spinTaps;
    private final JCheckBox bandStop;

    private WindowType windowType;
    private Window window;

    public FilterDialog(GenericFirFilter<?> filter) {
        super(Drtd.getMainGui().getPipelineDialog());
        this.filter = filter;
        final ListenerImpl listener = new ListenerImpl();

        setTitle("Configure filter");
        setResizable(true);
        setLocation(getParent().getLocation());
        setIconImages(Drtd.ICONS);
        setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
        setMinimumSize(new Dimension(400, 400));

        window = filter.getWindow();
        windowType = WindowType.getByInstance(window);
        if (windowType == null) {
            LOGGER.severe("Unknown filter \"" + window + "\"!");
            Utils.die();
        }

        final boolean isKaiser = windowType == WindowType.KAISER;
        plot = new FilterPlot(filter.getInputSampleRate());

        var rootPanel = new JPanel(new BorderLayout()); //Outermost panel
        rootPanel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
        rootPanel.add(plot, BorderLayout.CENTER);

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
            recalculate();
        });
        windowSelector.setSelectedItem(filter.getWindow());

        spinStart = new JSpinner(new SpinnerNumberModel(filter.getStartFrequency(),
                0,
                filter.getStopFrequency(),
                1));
        spinStart.addChangeListener(listener);

        spinEnd = new JSpinner(new SpinnerNumberModel(filter.getStopFrequency(),
                filter.getStartFrequency(),
                filter.getInputSampleRate() / 2,
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
        recalculate();
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

        recalculate();
    }

    private void recalculate() {
        /* Calculate FFT */
        final FloatFFT_1D fft = new FloatFFT_1D(FFTSIZE);
        final float[] coefficients = filter.getCoefficients();
        final float[] farr = new float[FFTSIZE * 2];
        final float[] res = new float[FFTSIZE / 2];
        float maxMagnitude = Float.MIN_VALUE;
        float minMagnitude = Float.MAX_VALUE;

        for (int i = 0; i < coefficients.length; ++i)
            farr[i * 2] = coefficients[i];

        fft.complexForward(farr);

        /* Calculate magnitude and find maximum */
        for (int i = 0; i < res.length; ++i) {
            final float magnitude = new Complex(farr[i * 2], farr[i * 2 + 1]).magnitude();

            res[i] = magnitude;
            maxMagnitude = Math.max(magnitude, maxMagnitude);
            minMagnitude = Math.min(magnitude, minMagnitude);
        }

        plot.plot(res, minMagnitude, maxMagnitude);
    }

    private enum WindowType {
        RECTANGULAR(RectangularWindow::new, RectangularWindow.class),
        BLACKMAN(BlackmanWindow::new, BlackmanWindow.class),
        HAMMING(HammingWindow::new, HammingWindow.class),
        HANN(HannWindow::new, HannWindow.class),
        KAISER(KaiserWindow::new, KaiserWindow.class);

        private final Provider<Window> constructor;
        private final Class<? extends Window> classObj;


        WindowType(Provider<Window> constructor, Class<? extends Window> classObj) {
            this.constructor = constructor;
            this.classObj = classObj;
        }

        public static Window[] createWindowInstances() {
            return Arrays.stream(values()).map(w -> w.constructor.get()).toArray(Window[]::new);
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
            plot.plot(false);
        }

        @Override
        public void actionPerformed(ActionEvent actionEvent) {
            if (actionEvent.getSource().equals(bandStop))
                filter.setBandStop(bandStop.isSelected());

            recalculate();
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

            recalculate();
            bandStop.setEnabled(filter.getStartFrequency() != filter.getStopFrequency());
        }

        @Override
        public void mouseClicked(MouseEvent mouseEvent) {
            try {
                spinAtt.commitEdit();
                spinEnd.commitEdit();
                spinStart.commitEdit();
                spinTaps.commitEdit();
                recalculate();
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
