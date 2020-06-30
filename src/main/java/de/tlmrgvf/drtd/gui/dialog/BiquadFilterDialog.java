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
import de.tlmrgvf.drtd.dsp.component.biquad.GenericBiquadFilter;
import de.tlmrgvf.drtd.gui.component.FilterPlot;
import de.tlmrgvf.drtd.utils.Utils;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.*;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.text.ParseException;

public final class BiquadFilterDialog extends JFrame {
    private final GenericBiquadFilter<?> filter;
    private final JComboBox<GenericBiquadFilter.Type> types;
    private final JSpinner center;
    private final JSpinner qbws;
    private final JSpinner gain;
    private final JLabel qbwsLabel;
    private final JLabel gainLabel;
    private final FilterPlot plot;

    public BiquadFilterDialog(GenericBiquadFilter<?> filter) {
        this.filter = filter;
        final ListenerImpl listener = new ListenerImpl();

        setTitle("Configure filter");
        setResizable(true);
        setIconImages(Drtd.ICONS);
        setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);
        setMinimumSize(new Dimension(400, 400));

        var sampleRate = filter.getSampleRate();
        plot = new FilterPlot(sampleRate);

        var rootPanel = new JPanel(new BorderLayout()); //Outermost panel
        rootPanel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
        rootPanel.add(plot, BorderLayout.CENTER);

        var gridLayout = new GridLayout(0, 2);
        gridLayout.setVgap(10);
        gridLayout.setHgap(10);
        JPanel settingsPanel = new JPanel(gridLayout);
        settingsPanel.setBorder(Utils.createLabeledBorder("Filter settings"));

        rootPanel.add(settingsPanel, BorderLayout.SOUTH);

        types = new JComboBox<>(GenericBiquadFilter.Type.values());
        center = new JSpinner(new SpinnerNumberModel(filter.getCenter(), 0, sampleRate / 2F, 10));
        qbws = new JSpinner(new SpinnerNumberModel(filter.getQbws(), 0, sampleRate / 2F, 1));
        gain = new JSpinner(new SpinnerNumberModel(filter.getGain(), -1000, 1000, .5));

        types.addItemListener(listener);
        center.addChangeListener(listener);
        qbws.addChangeListener(listener);
        gain.addChangeListener(listener);
        plot.addMouseListener(listener);

        settingsPanel.add(new JLabel("Type:"));
        settingsPanel.add(types);
        settingsPanel.add(new JLabel("Center:"));
        settingsPanel.add(center);
        qbwsLabel = new JLabel(filter.getType().getQbwsLabel());
        settingsPanel.add(qbwsLabel);
        settingsPanel.add(qbws);
        gainLabel = new JLabel("Gain:");
        settingsPanel.add(gainLabel);
        settingsPanel.add(gain);

        setSize(400, 400);
        add(rootPanel);
        updateFromFilter();
        pack();
        setLocationRelativeTo(null);
        recalculate();
    }

    private void recalculate() {
        /* Calculate magnitude and find maximum */
        final var coeff = filter.getCoefficients();
        final float[] res = new float[filter.getSampleRate() / 2];
        float minMagnitude = Float.MAX_VALUE;
        float maxMagnitude = Float.MIN_VALUE;
        float last = 0;
        for (int f = 0; f < res.length; ++f) {
            var magnitude = coeff.frequencyResponse(f + 1);
            if (magnitude == 0)
                magnitude = last;
            res[f] = magnitude;

            maxMagnitude = Math.max(magnitude, maxMagnitude);
            minMagnitude = Math.min(magnitude, minMagnitude);
            last = magnitude;
        }

        plot.plot(res, minMagnitude, maxMagnitude);
    }

    public void updateFromFilter() {
        gain.setValue((double) filter.getGain());
        types.setSelectedItem(filter.getType());
        center.setValue((double) filter.getCenter());
        qbwsLabel.setText(filter.getType().getQbwsLabel());
        qbws.setValue((double) filter.getQbws());
        gain.setEnabled(filter.getType().usesGain());
        gainLabel.setEnabled(gain.isEnabled());

        plot.plot(true);
    }

    private class ListenerImpl extends MouseAdapter implements ChangeListener, ItemListener {
        private void update() {
            filter.update((GenericBiquadFilter.Type) types.getSelectedItem(),
                    (float) (double) center.getValue(),
                    (float) (double) qbws.getValue(),
                    (float) (double) gain.getValue());
            gain.setEnabled(filter.getType().usesGain());
            gainLabel.setEnabled(gain.isEnabled());
            qbwsLabel.setText(filter.getType().getQbwsLabel());
            recalculate();
        }

        @Override
        public void itemStateChanged(ItemEvent itemEvent) {
            update();
        }

        @Override
        public void stateChanged(ChangeEvent changeEvent) {
            update();
        }

        @Override
        public void mouseClicked(MouseEvent mouseEvent) {
            try {
                gain.commitEdit();
                qbws.commitEdit();
                center.commitEdit();
                update();
            } catch (ParseException e) {
                JOptionPane.showMessageDialog(BiquadFilterDialog.this,
                        String.format("Invalid entry at position %d!", e.getErrorOffset() + 1), "Format error",
                        JOptionPane.ERROR_MESSAGE);
            }
        }
    }
}
