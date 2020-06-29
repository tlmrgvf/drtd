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

package de.tlmrgvf.drtd.decoder;

import de.tlmrgvf.drtd.decoder.utils.Marker;
import de.tlmrgvf.drtd.decoder.utils.MarkerGroup;
import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.dsp.component.BitConverter;
import de.tlmrgvf.drtd.dsp.component.IQMixer;
import de.tlmrgvf.drtd.dsp.component.Mapper;
import de.tlmrgvf.drtd.dsp.component.Normalizer;
import de.tlmrgvf.drtd.dsp.component.movingaverage.ComplexMovingAverage;
import de.tlmrgvf.drtd.dsp.pipeline.PipelineBuilder;
import de.tlmrgvf.drtd.gui.component.XYScope;
import de.tlmrgvf.drtd.utils.Complex;
import de.tlmrgvf.drtd.utils.Utils;
import de.tlmrgvf.drtd.utils.structure.BitBuffer;

import javax.swing.*;
import javax.swing.border.EtchedBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public final class Rtty extends Decoder<Boolean> {
    private final static int SAMPLE_RATE = 7350;
    private final static BaudotCode[] BAUDOT_CODES = new BaudotCode[32];
    private final static char FIGURES = 0x1B;
    private final static char LETTERS = 0x1F;
    private final static float SCOPE_PHASE_STEP = (float) (2 * Math.PI * (1000F / SAMPLE_RATE));

    static {
        /* ITA 2 code (US-TTY) */
        BAUDOT_CODES[0x00] = new BaudotCode("", "");
        BAUDOT_CODES[0x01] = new BaudotCode("E", "3");
        BAUDOT_CODES[0x02] = new BaudotCode("\n", "\n");
        BAUDOT_CODES[0x03] = new BaudotCode("A", "-");
        BAUDOT_CODES[0x04] = new BaudotCode(" ", " ");
        BAUDOT_CODES[0x05] = new BaudotCode("S", "<BEL>");
        BAUDOT_CODES[0x06] = new BaudotCode("I", "8");
        BAUDOT_CODES[0x07] = new BaudotCode("U", "7");
        BAUDOT_CODES[0x08] = new BaudotCode("<CR>", "<CR>", "\r");
        BAUDOT_CODES[0x09] = new BaudotCode("D", "$");
        BAUDOT_CODES[0x0A] = new BaudotCode("R", "4");
        BAUDOT_CODES[0x0B] = new BaudotCode("J", "'");
        BAUDOT_CODES[0x0C] = new BaudotCode("N", ",");
        BAUDOT_CODES[0x0D] = new BaudotCode("F", "!");
        BAUDOT_CODES[0x0E] = new BaudotCode("C", ":");
        BAUDOT_CODES[0x0F] = new BaudotCode("K", "(");
        BAUDOT_CODES[0x10] = new BaudotCode("T", "5");
        BAUDOT_CODES[0x11] = new BaudotCode("Z", "\"");
        BAUDOT_CODES[0x12] = new BaudotCode("L", ")");
        BAUDOT_CODES[0x13] = new BaudotCode("W", "2");
        BAUDOT_CODES[0x14] = new BaudotCode("H", "#");
        BAUDOT_CODES[0x15] = new BaudotCode("Y", "6");
        BAUDOT_CODES[0x16] = new BaudotCode("P", "0");
        BAUDOT_CODES[0x17] = new BaudotCode("Q", "1");
        BAUDOT_CODES[0x18] = new BaudotCode("O", "9");
        BAUDOT_CODES[0x19] = new BaudotCode("B", "?");
        BAUDOT_CODES[0x1A] = new BaudotCode("G", "&");
        /* FIGURES is 0x1B */
        BAUDOT_CODES[0x1C] = new BaudotCode("M", ".");
        BAUDOT_CODES[0x1D] = new BaudotCode("X", "/");
        BAUDOT_CODES[0x1E] = new BaudotCode("V", ";");
        /* LETTERS is 0x1F */
    }

    private final BitBuffer inputBuffer = new BitBuffer(true, 7);
    private final XYScope scope = new XYScope(100, true, 200, .2F, .8F);
    private final JTextArea outputTextArea = new JTextArea();
    private final JSpinner shiftSpinner;
    private final JSpinner baudrateSpinner;
    private final JCheckBox usbLsbCheckbox;
    private final JCheckBox escapeCrCheckbox;
    private final JCheckBox showTuningCheckbox;
    private final IQMixer spaceMixer;
    private final IQMixer markMixer;
    private final BitConverter converter;
    private final ComplexMovingAverage spaceFilter;
    private final ComplexMovingAverage markFilter;
    private final Normalizer spaceNormalizer;
    private final Normalizer markNormalizer;
    private JPanel tuningPanel;
    private MarkerGroup marker;
    private boolean waitStart = true;
    private boolean figures = false;
    private int shift = 170;
    private double baudrate = 45.45;
    private double scopePhase;

    public Rtty() {
        super(Boolean.class, SAMPLE_RATE);

        markMixer = new IQMixer(1);
        spaceMixer = new IQMixer(1);
        converter = new BitConverter(1);
        spaceFilter = new ComplexMovingAverage(1);
        markFilter = new ComplexMovingAverage(1);
        spaceNormalizer = new Normalizer(true, 1);
        markNormalizer = new Normalizer(true, 1);

        usbLsbCheckbox = new JCheckBox("Use LSB");
        escapeCrCheckbox = new JCheckBox("Print CR's");
        showTuningCheckbox = new JCheckBox("Show tuning");

        updateMarker();
        getManager().mapOption(Integer.class, this::getShift, this::setShift, 170)
                .mapOption(Double.class, this::getBaudrate, this::setBaudrate, 45.45)
                .mapOption(Integer.class, this::getCenterFrequency, this::setCenterFrequency, 0)
                .mapOption(Boolean.class, usbLsbCheckbox::isSelected, usbLsbCheckbox::setSelected, false)
                .mapOption(Boolean.class, escapeCrCheckbox::isSelected, escapeCrCheckbox::setSelected, false)
                .mapOption(Boolean.class, showTuningCheckbox::isSelected, showTuningCheckbox::setSelected, false)
                .loadAll();

        shiftSpinner = new JSpinner(new SpinnerNumberModel(shift, 10, 1500, 10));
        baudrateSpinner = new JSpinner(new SpinnerNumberModel(baudrate, 40, 300, 5));

        var listener = new ListenerImpl();
        shiftSpinner.addChangeListener(listener);
        baudrateSpinner.addChangeListener(listener);
        showTuningCheckbox.addActionListener(listener);

        outputTextArea.setLineWrap(true);
        outputTextArea.setEditable(false);
        outputTextArea.setFont(Utils.FONT);
        Utils.setupSmartAutoscroll(outputTextArea);
        Utils.addClearContextMenu(outputTextArea, () -> outputTextArea.setText(""));
    }

    @Override
    protected void onSetup() {
        updateMarker();
        updateFilters();
        updateMixers();
    }

    @Override
    protected PipelineComponent<Float, Boolean> buildPipeline() {
        var mark = markMixer.pipeline()
                .then(markFilter)
                .then(new Mapper<>(Float.class, Complex::magnitudeSquared))
                .build(markNormalizer);

        var space = spaceMixer.pipeline()
                .then(spaceFilter)
                .then(new Mapper<>(Float.class, Complex::magnitudeSquared))
                .build(spaceNormalizer);

        return PipelineBuilder.createEmpty(Float.class)
                .split(Float.class, i -> {
                    updateScope(i[0], i[1]);
                    return i[0] - i[1];
                }, mark, space)
                .then(new Mapper<>(Boolean.class, m -> m > 0))
                .build(converter);
    }

    @Override
    public void addGuiComponents(JPanel parent) {
        parent.setLayout(new BorderLayout());

        var topPanel = new JPanel();
        topPanel.setLayout(new BoxLayout(topPanel, BoxLayout.X_AXIS));

        var topLeftPanel = new JPanel(new FlowLayout(FlowLayout.LEADING));
        var settingsPanel = new JPanel(new GridLayout(0, 2, 2, 5));
        settingsPanel.add(new JLabel("Baudrate:"));
        settingsPanel.add(baudrateSpinner);
        settingsPanel.add(new JLabel("Shift:"));
        settingsPanel.add(shiftSpinner);
        settingsPanel.add(usbLsbCheckbox);
        settingsPanel.add(escapeCrCheckbox);
        settingsPanel.add(showTuningCheckbox);
        topLeftPanel.add(settingsPanel);

        tuningPanel = new JPanel();
        tuningPanel.setVisible(showTuningCheckbox.isSelected());
        tuningPanel.setLayout(new BoxLayout(tuningPanel, BoxLayout.X_AXIS));
        tuningPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED),
                "Tuning"));
        tuningPanel.add(scope);

        topPanel.add(topLeftPanel);
        topPanel.add(Box.createHorizontalGlue());
        topPanel.add(tuningPanel);

        var scrollPane = new JScrollPane(
                outputTextArea,
                ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER
        );
        scrollPane.setBorder(BorderFactory.createLoweredBevelBorder());
        outputTextArea.setBackground(parent.getBackground());

        parent.add(topPanel, BorderLayout.NORTH);
        parent.add(scrollPane, BorderLayout.CENTER);
    }

    @Override
    public void onMarkerMoved(int centerFrequency) {
        updateMixers();
    }

    private void updateMarker() {
        int center = marker == null ? 0 : marker.getCenter();
        marker = new MarkerGroup(true,
                new Marker(-shift / 2, (int) baudrate),
                new Marker(+shift / 2, (int) baudrate)
        );
        marker.setCenter(center);
        setMarker(marker);
    }

    private void updateFilters() {
        converter.setBaudRates((float) baudrate);
        final int samplesPerBit = (int) Math.round(SAMPLE_RATE / baudrate);

        spaceFilter.setTaps(samplesPerBit);
        markFilter.setTaps(samplesPerBit);

        /* At least 6 bits can be zero */
        spaceNormalizer.setWindowSize(samplesPerBit * 7);
        markNormalizer.setWindowSize(samplesPerBit * 7);
    }

    private void updateMixers() {
        spaceMixer.setFrequency(marker.getCenter() - shift / 2F);
        markMixer.setFrequency(marker.getCenter() + shift / 2F);
    }

    private void updateScope(Float mark, Float space) {
        if (!showTuningCheckbox.isSelected())
            return;

        scopePhase += SCOPE_PHASE_STEP;
        scopePhase %= 2 * Math.PI;

        scope.process(new Complex((float) (mark * Math.cos(scopePhase)), (float) (space * Math.sin(scopePhase))));
    }

    @Override
    protected void onPipelineResult(Boolean result) {
        /*
         *   RTTY is "idle on mark"
         *   Start bit  5 Baudot bits  1, 1.5 or 2 stop bits
         *   0          XXXXX          1(1)
         */
        result = result ^ usbLsbCheckbox.isSelected();
        inputBuffer.push(result);

        if (waitStart && !inputBuffer.get(0) && inputBuffer.get(6)) {
            waitStart = false;
            inputBuffer.resetBitCounter();
        } else if (waitStart || !inputBuffer.bitsAligned()) {
            return;
        }

        if (inputBuffer.get(0) || !inputBuffer.get(6)) {
            waitStart = true;
            return;
        }

        char bits = (char) ((inputBuffer.getBuffer() & 0x3F) >> 1);
        if (bits == LETTERS) {
            figures = false;
            return;
        } else if (bits == FIGURES) {
            figures = true;
            return;
        }

        BaudotCode baudotCode = BAUDOT_CODES[bits];
        String toAdd;
        if (baudotCode.nonEscaped == null || !escapeCrCheckbox.isSelected())
            toAdd = figures ? baudotCode.figure : baudotCode.letter;
        else
            toAdd = baudotCode.nonEscaped;

        Utils.doSmartAutoscroll(outputTextArea, toAdd);
    }

    private int getShift() {
        return shift;
    }

    private void setShift(int shift) {
        this.shift = shift;
    }

    private double getBaudrate() {
        return baudrate;
    }

    private void setBaudrate(double baudrate) {
        this.baudrate = baudrate;
    }

    private static class BaudotCode {

        private final String letter;
        private final String figure;
        private final String nonEscaped;

        public BaudotCode(String letter, String figure) {
            this.letter = letter;
            this.figure = figure;
            nonEscaped = null;
        }

        public BaudotCode(String letter, String figure, String nonEscaped) {
            this.letter = letter;
            this.figure = figure;
            this.nonEscaped = nonEscaped;
        }
    }

    private class ListenerImpl implements ChangeListener, ActionListener {
        @Override
        public void actionPerformed(ActionEvent actionEvent) {
            var source = actionEvent.getSource();
            if (source == showTuningCheckbox)
                tuningPanel.setVisible(showTuningCheckbox.isSelected());
        }

        @Override
        public void stateChanged(ChangeEvent changeEvent) {
            var source = changeEvent.getSource();
            if (source == baudrateSpinner) {
                setBaudrate((Double) baudrateSpinner.getValue());
            } else if (source == shiftSpinner) {
                setShift((int) shiftSpinner.getValue());
            }

            updateFilters();
            updateMarker();
        }
    }
}
