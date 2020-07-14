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
import de.tlmrgvf.drtd.dsp.window.Window;
import de.tlmrgvf.drtd.dsp.window.*;
import de.tlmrgvf.drtd.gui.component.Waterfall;
import de.tlmrgvf.drtd.gui.component.WaterfallPalette;
import de.tlmrgvf.drtd.utils.Utils;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.*;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

public final class WaterfallDialog extends JFrame {
    private final static int MIN_BINS_EXP = 10;
    private final static int MAX_BINS_EXP = 16;
    public final static int MAX_ZOOM = 10;

    private static WaterfallDialog instance;

    private final Waterfall waterfall;
    private final JComboBox<Window> windowSelector;
    private final JSlider zoom;
    private final JSlider speedMultiplier;
    private final JSpinner fStart;
    private final JComboBox<Integer> bins;
    private final JComboBox<WaterfallPalette> palette;
    private final JCheckBox powerSpectrum;
    private final SpinnerNumberModel fStartModel;

    private boolean ignoreUpdates = false;

    private WaterfallDialog(Waterfall waterfall) {
        this.waterfall = waterfall;
        instance = this;

        setTitle("Waterfall settings");
        setResizable(false);
        setIconImages(Drtd.ICONS);
        setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);

        GridLayout gridLayout = new GridLayout(0, 2);
        JPanel rootPanel = new JPanel();
        JPanel settingsPanel = new JPanel(gridLayout);
        rootPanel.setLayout(new BoxLayout(rootPanel, BoxLayout.Y_AXIS));
        settingsPanel.setBorder(Utils.createLabeledBorder("Settings"));
        rootPanel.add(settingsPanel);
        gridLayout.setVgap(10);
        gridLayout.setHgap(10);
        rootPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

        windowSelector = new JComboBox<>(new Window[]{
                new RectangularWindow(),
                new BlackmanWindow(),
                new HammingWindow(),
                new HannWindow()
        });

        speedMultiplier = new JSlider(1, 10);
        speedMultiplier.setMajorTickSpacing(1);
        speedMultiplier.setPaintTicks(true);

        zoom = new JSlider(-MAX_ZOOM * 10 + 10, MAX_ZOOM * 10 - 10, 0);
        zoom.setPaintTicks(true);
        zoom.setSnapToTicks(true);
        zoom.setMajorTickSpacing(1);

        fStart = new JSpinner();
        fStartModel = new SpinnerNumberModel();
        fStart.setModel(fStartModel);
        fStartModel.setMinimum(0);
        fStartModel.setMaximum(22050);

        Integer[] powersOfTwo = new Integer[MAX_BINS_EXP - MIN_BINS_EXP + 1];
        for (int i = MIN_BINS_EXP; i <= MAX_BINS_EXP; ++i)
            powersOfTwo[i - MIN_BINS_EXP] = (int) Math.pow(2, i);

        bins = new JComboBox<>(new DefaultComboBoxModel<>(powersOfTwo));
        powerSpectrum = new JCheckBox();
        palette = new JComboBox<>(WaterfallPalette.values());

        add(rootPanel);
        settingsPanel.add(new JLabel("Window:"));
        settingsPanel.add(windowSelector);
        settingsPanel.add(new JLabel("Zoom:"));
        settingsPanel.add(zoom);
        settingsPanel.add(new JLabel("f start:"));
        settingsPanel.add(fStart);
        settingsPanel.add(new JLabel("Speed multiplier:"));
        settingsPanel.add(speedMultiplier);
        settingsPanel.add(new JLabel("Bins:"));
        settingsPanel.add(bins);
        settingsPanel.add(new JLabel("Power spectrum:"));
        settingsPanel.add(powerSpectrum);
        settingsPanel.add(new JLabel("Palette:"));
        settingsPanel.add(palette);

        updateUiFromWaterfall();

        ListenerImpl listener = new ListenerImpl();
        windowSelector.addItemListener(listener);
        zoom.addChangeListener(listener);
        fStart.addChangeListener(listener);
        bins.addItemListener(listener);
        speedMultiplier.addChangeListener(listener);
        powerSpectrum.addChangeListener(listener);
        palette.addItemListener(listener);
        zoom.addMouseListener(listener);

        pack();
        setLocationRelativeTo(Drtd.getMainGui());
        Utils.addEscapeHandler(rootPanel, this);
    }

    public static WaterfallDialog getInstance(Waterfall waterfall) {
        if (instance != null)
            return instance;

        return new WaterfallDialog(waterfall);
    }

    public void updateUiFromWaterfall() {
        ignoreUpdates = true;

        windowSelector.setSelectedItem(waterfall.getWindow());
        final var zoomFactor = (waterfall.getZoom() - 1) * 10;
        zoom.setValue((int) ((waterfall.isZoomOut() ? -zoomFactor : zoomFactor)));
        fStart.setValue(waterfall.getFrequencyOffset());
        bins.setSelectedItem(waterfall.getBins());
        speedMultiplier.setValue(waterfall.getSpeedMultiplier());
        fStartModel.setMaximum(waterfall.getSampleRate() / 2);
        powerSpectrum.setSelected(waterfall.isPowerSpectrum());
        palette.setSelectedItem(waterfall.getPalette());

        ignoreUpdates = false;
    }

    @Override
    public void setVisible(boolean b) {
        super.setVisible(b);
        setState(Frame.NORMAL);
    }

    public void updateWaterfallFromUi() {
        assert bins.getSelectedItem() != null;

        waterfall.setBins((Integer) bins.getSelectedItem());
        waterfall.setZoom(Math.abs(zoom.getValue()) / 10F + 1);
        waterfall.setZoomOut(zoom.getValue() < 0);
        waterfall.setWindow(windowSelector.getItemAt(windowSelector.getSelectedIndex()));
        waterfall.setFrequencyOffset((Integer) fStart.getValue());
        waterfall.setSpeedMultiplier(speedMultiplier.getValue());
        waterfall.setPowerSpectrum(powerSpectrum.isSelected());
        waterfall.setPalette((WaterfallPalette) palette.getSelectedItem());
    }

    private class ListenerImpl extends MouseAdapter implements ItemListener, ChangeListener {
        @Override
        public void itemStateChanged(ItemEvent itemEvent) {
            updateWaterfallFromUi();
        }

        @Override
        public void stateChanged(ChangeEvent changeEvent) {
            if (!ignoreUpdates)
                updateWaterfallFromUi();
        }

        @Override
        public void mouseClicked(MouseEvent e) {
            if (e.getClickCount() == 2)
                zoom.setValue(0);
        }
    }
}
