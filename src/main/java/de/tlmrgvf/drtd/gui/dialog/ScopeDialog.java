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
import de.tlmrgvf.drtd.gui.component.Scope;
import de.tlmrgvf.drtd.utils.Utils;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

public final class ScopeDialog extends JFrame {
    private static ScopeDialog instance;

    private final Scope scope;
    private final JSlider sampleRate;
    private final JCheckBox removeDcBias;
    private final JCheckBox freerun;
    private final JCheckBox normalize;
    private final JToggleButton paused;
    private final JButton capture;
    private final JSlider triggering;

    private boolean ignoreUpdates = false;

    private ScopeDialog(Scope scope) {
        this.scope = scope;
        instance = this;
        setTitle("Scope settings");
        setResizable(false);
        setIconImage(Drtd.ICON);
        setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);

        GridLayout gridLayout = new GridLayout(0, 2);
        JPanel rootPanel = new JPanel();
        JPanel settingsPanel = new JPanel(gridLayout);

        rootPanel.setLayout(new BoxLayout(rootPanel, BoxLayout.Y_AXIS));
        settingsPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(), "Settings"));
        rootPanel.add(settingsPanel);
        gridLayout.setVgap(10);
        gridLayout.setHgap(10);
        rootPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

        sampleRate = new JSlider(JSlider.HORIZONTAL, -50, 50, 0);
        triggering = new JSlider();
        paused = new JToggleButton("Pause");
        capture = new JButton("Capture");
        freerun = new JCheckBox();
        removeDcBias = new JCheckBox();
        normalize = new JCheckBox();

        add(rootPanel);
        settingsPanel.add(new JLabel("Samples:"));
        settingsPanel.add(sampleRate);
        settingsPanel.add(new JLabel("Trigger:"));
        settingsPanel.add(triggering);
        settingsPanel.add(new JLabel("Freerun:"));
        settingsPanel.add(freerun);
        settingsPanel.add(new JLabel("Remove DC-Bias:"));
        settingsPanel.add(removeDcBias);
        settingsPanel.add(new JLabel("Normalize:"));
        settingsPanel.add(normalize);
        settingsPanel.add(paused);
        settingsPanel.add(capture);

        sampleRate.setPaintTicks(true);
        sampleRate.setMajorTickSpacing(1);

        triggering.setPaintTicks(true);
        triggering.setMajorTickSpacing(10);
        triggering.setMinorTickSpacing(5);

        loadFromScope();

        final ListenerImpl listener = new ListenerImpl();
        sampleRate.addChangeListener(listener);
        sampleRate.addMouseListener(listener);
        triggering.addMouseListener(listener);
        triggering.addChangeListener(listener);
        removeDcBias.addChangeListener(listener);
        freerun.addChangeListener(listener);
        normalize.addChangeListener(listener);
        capture.addActionListener((e) -> scope.captureOnce());
        paused.addActionListener((e) -> {
            boolean selected = paused.isSelected();
            scope.setPaused(selected);
            capture.setEnabled(selected);
        });
        capture.setEnabled(false);

        Utils.addEscapeHandler(rootPanel, this);
        pack();

        setLocationRelativeTo(Drtd.getMainGui().getFrame());
    }

    public static ScopeDialog getInstance(Scope scope) {
        if (instance != null)
            return instance;

        return new ScopeDialog(scope);
    }

    public void loadFromScope() {
        ignoreUpdates = true;

        sampleRate.setValue(scope.getSampleRatio());
        triggering.setValue(scope.getTriggerPosition());
        freerun.setSelected(scope.isFreerun());
        removeDcBias.setSelected(scope.isRemoveDcBias());
        triggering.setEnabled(!freerun.isSelected());
        normalize.setSelected(scope.isNormalize());

        ignoreUpdates = false;
    }

    @Override
    public void setVisible(boolean b) {
        super.setVisible(b);
        setState(NORMAL);
    }

    private void update() {
        triggering.setEnabled(!freerun.isSelected());
        scope.setSampleRatio(sampleRate.getValue());
        scope.setTriggerPosition(triggering.getValue());
        scope.setFreerun(freerun.isSelected());
        scope.setRemoveDcBias(removeDcBias.isSelected());
        scope.setNormalize(normalize.isSelected());
    }

    private class ListenerImpl extends MouseAdapter implements ChangeListener {
        @Override
        public void stateChanged(ChangeEvent changeEvent) {
            if (!ignoreUpdates)
                update();
        }

        @Override
        public void mouseClicked(MouseEvent mouseEvent) {
            if (mouseEvent.getClickCount() == 2) {
                if (mouseEvent.getSource() == sampleRate)
                    sampleRate.setValue(0);
                else
                    triggering.setValue(50);

                update();
            }
        }
    }
}
