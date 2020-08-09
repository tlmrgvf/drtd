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

package de.tlmrgvf.drtd.decoder.pocsag;


import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.decoder.HeadlessDecoder;
import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.dsp.component.BitConverter;
import de.tlmrgvf.drtd.dsp.component.Mapper;
import de.tlmrgvf.drtd.dsp.component.firfilter.FirFilter;
import de.tlmrgvf.drtd.dsp.component.movingaverage.MovingAverage;
import de.tlmrgvf.drtd.dsp.window.HammingWindow;
import de.tlmrgvf.drtd.gui.component.LabeledIndicator;
import de.tlmrgvf.drtd.utils.Utils;
import de.tlmrgvf.drtd.utils.bch.BchCode;
import de.tlmrgvf.drtd.utils.bch.Z2Polynomial;
import de.tlmrgvf.drtd.utils.structure.BitBuffer;

import javax.swing.*;
import javax.swing.border.EtchedBorder;
import java.awt.*;
import java.util.Arrays;
import java.util.logging.Logger;
import java.util.stream.Collectors;

public final class Pocsag extends HeadlessDecoder<Boolean, PocsagMessage> {
    private final static BchCode BCH_CODE = new BchCode(
            BchCode.EncodingType.PREFIX,
            new Z2Polynomial(0b11101101001), //x^10 + x^9 + x^8 + x^6 + x^5 + x^3 + 1
            31,
            21,
            2
    );
    private final static int SAMPLE_RATE = 12000;
    private final static int INPUT_FILTER_CUTOFF_DISTANCE = 200;
    private final static int INPUT_FILTER_INITIAL_CUTOFF = 2400 + INPUT_FILTER_CUTOFF_DISTANCE;
    private final static Logger LOGGER = Drtd.getLogger(Pocsag.class);

    private final JComboBox<PocsagMessage.ContentType> contentSelector;
    private final LabeledIndicator dataIndicator;
    private final LabeledIndicator syncIndicator;
    private final MovingAverage filter;
    private final FirFilter inputFilter;
    private final BitConverter bitConverter;
    private final JTextArea outputTextArea;
    private PocsagMessage.ContentType selectetContentType = PocsagMessage.ContentType.BOTH;
    private State state = State.FIRST_BIT_SINCE_SYNC;
    private PocsagMessage.MessageBuilder messageBuilder;

    private final BitBuffer bitBuffer;
    private int preambleCount = 0;
    private int codewordCount = 0;
    private boolean receivedParity = false;
    private boolean inverted = false;
    private boolean lastBit = false;

    public Pocsag() {
        super(Boolean.class, SAMPLE_RATE);
        outputTextArea = new JTextArea();
        dataIndicator = new LabeledIndicator(LabeledIndicator.GREEN_ON_COLOR,
                LabeledIndicator.GREEN_OFF_COLOR,
                "Data");
        syncIndicator = new LabeledIndicator(LabeledIndicator.YELLOW_ON_COLOR,
                LabeledIndicator.YELLOW_OFF_COLOR, "Sync");
        filter = new MovingAverage(1);
        inputFilter = new FirFilter(new HammingWindow(), 51, 0, INPUT_FILTER_INITIAL_CUTOFF);
        bitConverter = new BitConverter(
                (float samplesPerBit, float bauds) -> {
                    filter.setTaps(Math.round(samplesPerBit));
                    inputFilter.setStopFrequency((int) bauds + INPUT_FILTER_CUTOFF_DISTANCE);
                },
                25,
                512, 1200, 2400);
        bitBuffer = new BitBuffer(false, PocsagData.CODEWORD_BITS);
        contentSelector = new JComboBox<>(PocsagMessage.ContentType.values());
        contentSelector.addActionListener((actionEvent) ->
                selectetContentType = (PocsagMessage.ContentType) contentSelector.getSelectedItem());
        getSettingsManager()
                .mapOption(Integer.class, contentSelector::getSelectedIndex, contentSelector::setSelectedIndex, 3)
                .loadAll();
        Utils.setupSmartAutoscroll(outputTextArea);
    }

    @Override
    protected void onSetup() {
        filter.setTaps(1);
        inputFilter.setStopFrequency(INPUT_FILTER_INITIAL_CUTOFF);
        bitConverter.waitForSync();
        reset();
    }

    @Override
    protected PipelineComponent<Float, Boolean> buildPipeline() {
        return inputFilter.pipeline()
                .then(filter)
                .then(new Mapper<>(Boolean.class, m -> m < 0))
                .build(bitConverter);
    }

    private void reset() {
        bitConverter.waitForSync();
        codewordCount = 0;
        inverted = false;
        bitBuffer.reset();
        preambleCount = 0;
        filter.setTaps(1);
        inputFilter.setStopFrequency(INPUT_FILTER_INITIAL_CUTOFF);
        dataIndicator.forceState(false);
        syncIndicator.forceState(false);
        updateState(State.FIRST_BIT_SINCE_SYNC);
        messageBuilder = new PocsagMessage.MessageBuilder();
    }

    @Override
    public boolean setupParameters(String[] args) {
        PocsagMessage.ContentType type = PocsagMessage.ContentType.fromName(args[0]);
        if (type == null)
            return false;

        selectetContentType = type;
        return true;
    }

    @Override
    public String[] getChangeableParameters() {
        return new String[]{Arrays.stream(PocsagMessage.ContentType.values())
                .map(PocsagMessage.ContentType::toString)
                .collect(Collectors.joining("/"))};
    }

    @Override
    protected void showResultInGui(PocsagMessage result) {
        Utils.doSmartAutoscroll(outputTextArea, result.toString());
    }

    @Override
    protected PocsagMessage calculateResult(Boolean inputBit) {
        PocsagMessage finalMessage = null;
        inputBit ^= inverted;
        receivedParity ^= inputBit;
        bitBuffer.push(inputBit);
        dataIndicator.setState(inputBit);

        switch (state) {
            case FIRST_BIT_SINCE_SYNC:
                lastBit = inputBit;
                updateState(State.WAIT_FOR_INITIAL_SYNC_WORD);
                break;
            case WAIT_FOR_INITIAL_SYNC_WORD:
                bitBuffer.resetBitCounter();
                if (inputBit == lastBit && preambleCount < PocsagData.PREAMBLE_COUNT / 4) {
                    LOGGER.fine("Invalid preamble!");
                    reset();
                    break;
                }

                lastBit = inputBit;

                if (++preambleCount > PocsagData.PREAMBLE_COUNT * 3) {
                    LOGGER.fine("Preamble too long!");
                    reset();
                    break;
                }
            case WAIT_FOR_IMMEDIATE_SYNC_WORD:
                if (state == State.WAIT_FOR_INITIAL_SYNC_WORD || bitBuffer.bitsAligned()) {
                    int word = (int) bitBuffer.getBuffer();
                    if (state == State.WAIT_FOR_IMMEDIATE_SYNC_WORD)
                        word = (int) BCH_CODE.correctCodeword((word >> 1) & ~0x80000000) << 1;

                    if (word == PocsagData.SYNC_WORD) {
                        LOGGER.fine("Sync...");
                    } else if (word == ~PocsagData.SYNC_WORD) {
                        LOGGER.fine("Inverted Sync detected, inverting all other bits from here on out!");
                    } else if (state == State.WAIT_FOR_IMMEDIATE_SYNC_WORD) {
                        LOGGER.fine("Did not get expected sync codeword, message done.");
                        finalMessage = messageBuilder.build(selectetContentType, (int) bitConverter.getBaudrate());
                        reset();
                        break;
                    } else {
                        break;
                    }

                    if (state == State.WAIT_FOR_INITIAL_SYNC_WORD)
                        inverted = word == ~PocsagData.SYNC_WORD;

                    syncIndicator.forceState(true);
                    updateState(State.READ_POCSAG_BATCH);
                    receivedParity = true;
                }

                break;
            case READ_POCSAG_BATCH:
                if (!bitBuffer.bitsAligned())
                    break;

                //POCSAG uses even parity
                if (!receivedParity)
                    LOGGER.fine("Parity error!");

                long corrected = BCH_CODE.correctCodeword((bitBuffer.getBuffer() >> 1) & ~0x80000000);
                if (corrected == BchCode.INVALID) {
                    messageBuilder.setContainsInvalidCodeword();
                } else {
                    PocsagData data = PocsagData.fromBits(codewordCount, (int) corrected);
                    messageBuilder.appendData(data);

                    if (data.getType() == PocsagData.Type.IDLE || data.getType() == PocsagData.Type.ADDRESS) {
                        if (messageBuilder.isValid())
                            finalMessage = messageBuilder.build(selectetContentType, (int) bitConverter.getBaudrate());
                        messageBuilder = new PocsagMessage.MessageBuilder();
                    }
                    ++codewordCount;
                }

                if (codewordCount == PocsagMessage.CODEWORDS_PER_BATCH) {
                    updateState(State.WAIT_FOR_IMMEDIATE_SYNC_WORD);
                    codewordCount = 0;
                }

                receivedParity = true;
                break;
        }

        return finalMessage;
    }

    private void updateState(State newState) {
        state = newState;
        setStatus(newState.toString());
    }

    @Override
    public void addGuiComponents(JPanel parent) {
        parent.setLayout(new BorderLayout());

        outputTextArea.setEditable(false);
        outputTextArea.setFont(Utils.FONT);
        outputTextArea.setBackground(parent.getBackground());
        outputTextArea.setLineWrap(false);

        Utils.addClearContextMenu(outputTextArea, () -> outputTextArea.setText(""));

        JScrollPane scroll = new JScrollPane(
                outputTextArea,
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED
        );

        scroll.setBorder(BorderFactory.createLoweredBevelBorder());
        parent.add(scroll);

        JPanel indicatorContainer = new JPanel();
        indicatorContainer.setLayout(new BoxLayout(indicatorContainer, BoxLayout.X_AXIS));
        indicatorContainer.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(2, 2, 2, 2),
                BorderFactory.createEtchedBorder(EtchedBorder.RAISED)));
        indicatorContainer.add(Box.createRigidArea(new Dimension(
                3,
                LabeledIndicator.DIMENSIONS.height + 10)));
        indicatorContainer.add(syncIndicator);
        indicatorContainer.add(Box.createHorizontalStrut(8));
        indicatorContainer.add(dataIndicator);
        indicatorContainer.add(Box.createHorizontalGlue());

        JPanel selectorPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        selectorPanel.setBorder(BorderFactory.createEmptyBorder(2, 2, 2, 2));
        selectorPanel.add(new JLabel("Print:"));
        selectorPanel.add(contentSelector);

        indicatorContainer.add(selectorPanel);
        parent.add(indicatorContainer, BorderLayout.NORTH);
        updateState(State.FIRST_BIT_SINCE_SYNC);
    }

    private enum State {
        FIRST_BIT_SINCE_SYNC("Waiting for preamble..."),
        WAIT_FOR_INITIAL_SYNC_WORD("Waiting for sync..."),
        WAIT_FOR_IMMEDIATE_SYNC_WORD("Waiting for immediate sync..."),
        READ_POCSAG_BATCH("Reading batch...");

        final String description;

        State(String description) {
            this.description = description;
        }

        @Override
        public String toString() {
            return description;
        }
    }
}
