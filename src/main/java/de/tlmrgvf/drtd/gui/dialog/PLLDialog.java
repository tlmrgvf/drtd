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
import de.tlmrgvf.drtd.dsp.component.PLL;
import de.tlmrgvf.drtd.gui.component.RollingScope;
import de.tlmrgvf.drtd.utils.DownSampler;
import de.tlmrgvf.drtd.utils.Utils;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.*;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;

public final class PLLDialog extends JDialog {
    private final PLL pll;
    private final RollingScope graph;
    private final JSpinner frequencySpinner;
    private final JSpinner gainSpinner;
    private final DownSampler downSampler;
    private final BiquadFilterDialog loopFilterDialog;
    private final BiquadFilterDialog lowpassFilterDialog;

    public PLLDialog(PLL pll) {
        super(Drtd.getMainGui().getPipelineDialog());
        this.pll = pll;
        downSampler = new DownSampler(pll.getInputSampleRate(), 200);
        ListenerImpl listener = new ListenerImpl();
        loopFilterDialog = new BiquadFilterDialog(pll.getLoopFilter());
        lowpassFilterDialog = new BiquadFilterDialog(pll.getLowpassFilter());

        setTitle("Configure PLL");
        setResizable(false);
        setLocation(getParent().getLocation());
        setIconImages(Drtd.ICONS);
        setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
        setMinimumSize(new Dimension(400, 300));
        setLayout(new BorderLayout());
        addWindowListener(listener);

        final JPanel graphPanel = new JPanel(new BorderLayout());
        graphPanel.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(4, 4, 4, 4),
                Utils.createLabeledBorder("Phase error")));
        graph = new RollingScope();
        graphPanel.add(graph);

        final JPanel settingsPanel = new JPanel(new GridLayout(0, 2, 4, 4));
        settingsPanel.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(4, 4, 4, 4),
                Utils.createLabeledBorder("Settings")));

        frequencySpinner = new JSpinner(new SpinnerNumberModel(pll.getFrequency(),
                0,
                pll.getInputSampleRate() / 2F,
                1));
        gainSpinner = new JSpinner(new SpinnerNumberModel(pll.getLoopGain(), 0, 1, .002));

        settingsPanel.add(new JLabel("Frequency:"));
        settingsPanel.add(frequencySpinner);
        settingsPanel.add(new JLabel("Loop gain:"));
        settingsPanel.add(gainSpinner);

        JButton loopFilter = new JButton("Loop filter");
        JButton lpFilter = new JButton("Lowpass filter");

        settingsPanel.add(loopFilter);
        settingsPanel.add(lpFilter);

        frequencySpinner.addChangeListener(listener);
        gainSpinner.addChangeListener(listener);
        loopFilter.addActionListener(actionEvent -> {
            loopFilterDialog.setVisible(true);
            loopFilterDialog.setState(JFrame.NORMAL);
        });
        lpFilter.addActionListener(actionEvent -> {
            lowpassFilterDialog.setVisible(true);
            lowpassFilterDialog.setState(JFrame.NORMAL);
        });

        add(graphPanel);
        add(settingsPanel, BorderLayout.SOUTH);
        pack();
    }

    public void updateDialog(float error) {
        Float downSampled = downSampler.sample(error);
        if (downSampled != null)
            graph.append(downSampled);
    }

    private class ListenerImpl implements ChangeListener, WindowListener {

        @Override
        public void stateChanged(ChangeEvent changeEvent) {
            Object source = changeEvent.getSource();
            if (source == frequencySpinner)
                pll.setFrequency((float) (double) frequencySpinner.getValue());
            else if (source == gainSpinner)
                pll.setLoopGain((float) (double) gainSpinner.getValue());
        }

        @Override
        public void windowOpened(WindowEvent windowEvent) {

        }

        @Override
        public void windowClosing(WindowEvent windowEvent) {
            loopFilterDialog.dispose();
            lowpassFilterDialog.dispose();
        }

        @Override
        public void windowClosed(WindowEvent windowEvent) {

        }

        @Override
        public void windowIconified(WindowEvent windowEvent) {

        }

        @Override
        public void windowDeiconified(WindowEvent windowEvent) {

        }

        @Override
        public void windowActivated(WindowEvent windowEvent) {

        }

        @Override
        public void windowDeactivated(WindowEvent windowEvent) {

        }
    }
}
