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

package de.tlmrgvf.drtd.gui;

import de.tlmrgvf.drtd.DecoderImplementation;
import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.decoder.Decoder;
import de.tlmrgvf.drtd.dsp.Interpreter;
import de.tlmrgvf.drtd.dsp.component.BiquadFilterComponent;
import de.tlmrgvf.drtd.dsp.component.firfilter.GenericFirFilter;
import de.tlmrgvf.drtd.gui.component.Scope;
import de.tlmrgvf.drtd.gui.component.Waterfall;
import de.tlmrgvf.drtd.gui.dialog.PipelineDialog;
import de.tlmrgvf.drtd.gui.utils.config.ConfigureDialog;
import de.tlmrgvf.drtd.utils.SettingsManager;

import javax.swing.*;
import javax.swing.border.EtchedBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.plaf.basic.BasicSplitPaneDivider;
import javax.swing.plaf.basic.BasicSplitPaneUI;
import java.awt.*;
import java.awt.event.*;
import java.util.logging.Logger;

public final class MainGui extends JFrame {
    private final static Logger LOGGER = Drtd.getLogger(MainGui.class);
    private final static SettingsManager SETTINGS_MANAGER = SettingsManager.createFor(MainGui.class);

    private final Waterfall waterfall;
    private final Scope scope;
    private final BasicSplitPaneDivider splitPaneDivider;
    private final JButton config;
    private final JComboBox<DecoderImplementation> decoderComboBox;
    private final JComboBox<String> shownValue;
    private final JPanel decoderPanel;
    private final JPanel contentPanel;
    private final PipelineDialog pipelineDialog;
    private final JLabel statusLabel;
    private final JSpinner frequencySpinner;
    private final JSplitPane rootSplitPane;

    private boolean pauseGui = false;
    private Interpreter valueInterpreter;

    public MainGui() {
        super(Drtd.NAME);

        setLayout(new BorderLayout());
        setMinimumSize(new Dimension(480, 240));
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setIconImages(Drtd.ICONS);

        ListenerImpl listener = new ListenerImpl();
        rootSplitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
        JSplitPane waveSplitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);

        addWindowListener(listener);
        rootSplitPane.setBorder(BorderFactory.createEmptyBorder(2, 2, 2, 2));
        add(rootSplitPane, BorderLayout.CENTER);

        contentPanel = new JPanel(new BorderLayout());
        JPanel waterfallPanel = new JPanel(new BorderLayout(0, 0));

        rootSplitPane.setLeftComponent(contentPanel);
        rootSplitPane.setRightComponent(waveSplitPane);

        decoderPanel = new JPanel();
        JPanel shortcutPanel = new JPanel();
        shortcutPanel.setLayout(new BoxLayout(shortcutPanel, BoxLayout.X_AXIS));
        shortcutPanel.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED));
        contentPanel.add(shortcutPanel, BorderLayout.NORTH);
        contentPanel.add(decoderPanel, BorderLayout.CENTER);

        config = new JButton("Configure...");
        config.addActionListener(listener);

        decoderComboBox = new JComboBox<>(DecoderImplementation.values());
        decoderComboBox.addItemListener(listener);

        var leftPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        var rightPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT));

        frequencySpinner = new JSpinner();
        updateFrequencySpinner(22050, 0);
        frequencySpinner.setEnabled(false);
        frequencySpinner.addChangeListener(listener);

        leftPanel.add(decoderComboBox);
        leftPanel.add(config);
        rightPanel.add(new JLabel("Center frequency:"));
        rightPanel.add(frequencySpinner);
        rightPanel.add(new JLabel("Hz"));

        shortcutPanel.add(leftPanel);
        shortcutPanel.add(Box.createHorizontalGlue());
        shortcutPanel.add(rightPanel);

        waterfall = new Waterfall(4096);
        scope = new Scope();
        waterfall.setToolTipText("Right click to configure");
        scope.setToolTipText("Right click to configure");
        JPanel contrastPanel = new JPanel();
        contrastPanel.setLayout(new BoxLayout(contrastPanel, BoxLayout.Y_AXIS));

        waveSplitPane.setLeftComponent(scope);
        waveSplitPane.setRightComponent(waterfallPanel);

        waterfallPanel.add(waterfall, BorderLayout.CENTER);
        waterfallPanel.add(contrastPanel, BorderLayout.EAST);

        pipelineDialog = new PipelineDialog();
        pack();

        splitPaneDivider = ((BasicSplitPaneUI) rootSplitPane.getUI()).getDivider();
        splitPaneDivider.addMouseListener(listener);
        waterfall.addMouseListener(listener);
        scope.addMouseListener(listener);

        JPanel statusBar = new JPanel(new BorderLayout(10, 0));
        statusBar.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(0, 2, 2, 2),
                BorderFactory.createCompoundBorder(
                        BorderFactory.createEtchedBorder(EtchedBorder.RAISED),
                        BorderFactory.createEmptyBorder(2, 2, 2, 2))));

        JPanel statusPanel = new JPanel();
        BoxLayout boxLayout = new BoxLayout(statusPanel, BoxLayout.X_AXIS);
        statusPanel.setLayout(boxLayout);

        statusLabel = new JLabel("Ready.");
        shownValue = new JComboBox<>(new String[]{"(Nothing available)"});
        shownValue.setEnabled(false);
        shownValue.setEditable(false);
        shownValue.addItemListener(listener);

        statusPanel.add(statusLabel);
        statusPanel.add(Box.createHorizontalGlue());

        statusBar.add(statusPanel, BorderLayout.CENTER);
        statusBar.add(shownValue, BorderLayout.WEST);
        add(statusBar, BorderLayout.SOUTH);

        waveSplitPane.setResizeWeight(0);
        rootSplitPane.setResizeWeight(1);

        SETTINGS_MANAGER
                .mapOption(Integer.class, waveSplitPane::getDividerLocation, waveSplitPane::setDividerLocation, 200)
                .mapOption(Integer.class, rootSplitPane::getDividerLocation, rootSplitPane::setDividerLocation, 300)
                .mapOption(Dimension.class, this::getSize, this::setSize, new Dimension(640, 480))
                .mapOption(Integer.class, decoderComboBox::getSelectedIndex, decoderComboBox::setSelectedIndex, 0);
    }

    public void loadSettings() {
        SETTINGS_MANAGER.loadAll();
    }

    public void updateStatus(String status) {
        statusLabel.setText(status);
    }

    public void updateDecoder() {
        var old = Drtd.getProcessingThread();
        if (old != null)
            old.getDecoder().saveSettings();

        final Decoder<?> decoder = ((DecoderImplementation) decoderComboBox.getSelectedItem()).getInstance();
        LOGGER.fine("Set new decoder " + decoder);
        assert decoder != null;

        Drtd.stopProcessing();

        updateStatus("Ready.");
        decoderPanel.removeAll();
        resetInterpreter();
        waterfall.setDecoder(decoder);

        decoder.setup();
        decoder.addGuiComponents(decoderPanel);
        decoderPanel.repaint();

        if (rootSplitPane.getMinimumDividerLocation() > rootSplitPane.getDividerLocation())
            rootSplitPane.setDividerLocation(rootSplitPane.getMinimumDividerLocation());

        pipelineDialog.setDecoder(decoder);
        ConfigureDialog.closeAll();
        GenericFirFilter.closeDialog();
        BiquadFilterComponent.closeDialog();
        Drtd.getUpdateThread().setDecoder(decoder);
        setMinimumSize(contentPanel.getMinimumSize());
        Drtd.startProcessing(decoder);
    }

    public void updateFrequencySpinner(int max, int frequency) {
        frequencySpinner.setModel(new SpinnerNumberModel(frequency, 0, max, 1));
        frequencySpinner.setValue(frequency);
    }

    public void enableFrequencySpinner(boolean enable) {
        frequencySpinner.setEnabled(enable);
    }

    public void resetInterpreter() {
        valueInterpreter = null;
    }

    public Waterfall getWaterfall() {
        return waterfall;
    }

    public PipelineDialog getPipelineDialog() {
        return pipelineDialog;
    }

    private void chooseInterpreter(Class<?> obj) {
        valueInterpreter = Drtd.getInterpreter(obj);
        LOGGER.fine("Use interpreter for " + obj.getName());

        if (valueInterpreter == null) {
            shownValue.setModel(new DefaultComboBoxModel<>(new String[]{"(Nothing available)"}));
            shownValue.setEnabled(false);
        } else {
            shownValue.setModel(new DefaultComboBoxModel<>(valueInterpreter.getViewableValues()));
            shownValue.setEnabled(true);
        }

        shownValue.setSelectedIndex(0);
    }

    public void updateMonitor(Object sample) {
        if (sample == null || waterfall == null || pauseGui)
            return;
        else if (valueInterpreter == null)
            chooseInterpreter(sample.getClass());

        Float value = valueInterpreter.interpret(sample);

        if (value != null) {
            scope.process(value);
            waterfall.process(value);
        }
    }

    public JFrame getFrame() {
        return this;
    }

    private class ListenerImpl extends MouseAdapter implements ActionListener, WindowListener, ItemListener, ChangeListener {

        @Override
        public void actionPerformed(ActionEvent event) {
            Object source = event.getSource();
            if (source.equals(config)) {
                pipelineDialog.setVisible(true);
                pipelineDialog.requestFocus();
            }
        }

        @Override
        public void windowClosing(WindowEvent windowEvent) {
            var old = Drtd.getProcessingThread();
            if (old != null)
                old.getDecoder().saveSettings();
            SettingsManager.saveManagers();
            SettingsManager.saveFile();
        }

        @Override
        public void mousePressed(MouseEvent e) {
            if (e.getComponent().equals(splitPaneDivider))
                pauseGui = true;
        }

        @Override
        public void mouseReleased(MouseEvent e) {
            if (e.getComponent().equals(splitPaneDivider))
                pauseGui = false;
        }

        @Override
        public void itemStateChanged(ItemEvent itemEvent) {
            if (itemEvent.getStateChange() == ItemEvent.SELECTED)
                if (itemEvent.getSource() == shownValue)
                    valueInterpreter.view(shownValue.getSelectedIndex());
                else
                    updateDecoder();
        }

        @Override
        public void stateChanged(ChangeEvent changeEvent) {
            int frequency = (Integer) frequencySpinner.getValue();

            var processingThread = Drtd.getProcessingThread();
            if (processingThread != null)
                processingThread.getDecoder().setCenterFrequency(frequency);
        }

        @Override
        public void windowOpened(WindowEvent windowEvent) {
            updateDecoder();
            setLocationRelativeTo(null);
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
