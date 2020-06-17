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

package de.tlmrgvf.drtd.decoder.ax25;

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.decoder.Decoder;
import de.tlmrgvf.drtd.decoder.ax25.protocol.Ax25Packet;
import de.tlmrgvf.drtd.decoder.utils.Marker;
import de.tlmrgvf.drtd.decoder.utils.MarkerGroup;
import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.dsp.component.*;
import de.tlmrgvf.drtd.dsp.component.firfilter.ComplexFirFilter;
import de.tlmrgvf.drtd.dsp.component.movingaverage.MovingAverage;
import de.tlmrgvf.drtd.dsp.window.HammingWindow;
import de.tlmrgvf.drtd.gui.component.LabeledIndicator;
import de.tlmrgvf.drtd.utils.Utils;
import de.tlmrgvf.drtd.utils.structure.BitBuffer;

import javax.swing.*;
import javax.swing.border.EtchedBorder;
import java.awt.*;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.logging.Logger;

public final class Ax25 extends Decoder<Boolean> {
    private final static int SAMPLE_RATE = 22050;
    private final static int BAUD_RATE = 1200;
    private final static int HEADERS_NEEDED = 5;
    private final static Logger LOGGER = Drtd.getLogger(Ax25.class);

    private final PacketTableModel model;
    private final JTable packetTable;
    private final LabeledIndicator syncIndicator;
    private final LabeledIndicator dataIndicator;
    private final BitBuffer inBuffer = new BitBuffer(true);
    private final BitBuffer delayBuffer = new BitBuffer(true);
    private final BitBuffer processedBuffer = new BitBuffer(true);
    private final List<Byte> packetBuffer = new LinkedList<>();

    private State state;
    private int oneCount, headerCount;

    public Ax25() {
        super(Boolean.class, SAMPLE_RATE);
        packetTable = new JTable();
        model = new PacketTableModel(packetTable);
        packetTable.setModel(model);
        packetTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

        dataIndicator = new LabeledIndicator(LabeledIndicator.GREEN_ON_COLOR,
                LabeledIndicator.GREEN_OFF_COLOR,
                "Data");
        syncIndicator = new LabeledIndicator(LabeledIndicator.YELLOW_ON_COLOR,
                LabeledIndicator.YELLOW_OFF_COLOR, "Sync");

        MarkerGroup markerGroup = new MarkerGroup(false,
                new Marker(-500, 400),
                new Marker(500, 400)
        );
        markerGroup.setCenter(1700);
        setMarker(markerGroup);
    }

    @Override
    public void onSetup() {
        changeState(State.WAIT_FLAG);
    }

    @Override
    protected PipelineComponent<Float, Boolean> buildPipeline() {
        return new IQMixer(1700).pipeline()
                .then(new ComplexFirFilter(new HammingWindow(), 41, 0, 600))
                .then(new AngleDifference())
                .then(new MovingAverage(Math.round(SAMPLE_RATE / (float) BAUD_RATE)))
                .then(new Mapper<>(Boolean.class, m -> m < 0))
                .then(new BitConverter(1200))
                .build(new NRZIDecoder(true));
    }

    @Override
    protected void onPipelineResult(Boolean result) {
        Boolean pbit = processBit(delayBuffer.push(inBuffer.push(result)));
        if (pbit != null) processedBuffer.push(pbit);

        char bte = (char) inBuffer.getBuffer();
        char pbte = (char) processedBuffer.getBuffer();

        if (state.updateIndicator)
            dataIndicator.setState(result);

        switch (state) {
            case WAIT_FLAG:
                if (bte == Ax25Packet.MAGIC_FLAG) {
                    changeState(State.COUNT_FLAG);
                    headerCount = 1;
                }
                break;
            case COUNT_FLAG:
                if (!inBuffer.bitsAligned()) return;

                if (bte == Ax25Packet.MAGIC_FLAG) {
                    if (++headerCount >= HEADERS_NEEDED) changeState(State.WAIT_DATA);
                } else {
                    headerCount = 0;
                    changeState(State.WAIT_FLAG);
                }
                break;
            case WAIT_DATA:
                syncIndicator.forceState(true);
                if (inBuffer.bitsAligned() && bte != Ax25Packet.MAGIC_FLAG) changeState(State.WAIT_END);
                break;
            case WAIT_END:
                if (bte == Ax25Packet.MAGIC_FLAG) {
                    changeState(State.WAIT_FLAG);
                    packetReceived();
                } else if (processedBuffer.bitsAligned()) {
                    packetBuffer.add((byte) pbte);
                }
                break;
        }
    }

    private void packetReceived() {
        dataIndicator.forceState(false);
        syncIndicator.forceState(false);

        Byte[] bytes = packetBuffer.toArray(Byte[]::new);
        if (bytes.length > 0) {
            Ax25Packet parse = Ax25Packet.parse(Arrays.copyOfRange(bytes, 1, bytes.length));
            if (parse != null) model.addPacket(parse);
        }
        packetBuffer.clear();
    }

    private void changeState(State next) {
        if (next.ignoreStuffed) {
            delayBuffer.reset();
            processedBuffer.reset();
            oneCount = 0;
        }

        inBuffer.resetBitCounter();
        state = next;
        setStatus(next.label);
    }

    private Boolean processBit(Boolean bit) {
        if (oneCount >= 5) {
            oneCount = 0;
            if (bit && state == State.WAIT_END) {
                LOGGER.fine("Expected zero is one, assuming packet is done?");
                packetReceived();
                changeState(State.WAIT_FLAG);
            }
            return null;
        } else if (bit) {
            ++oneCount;
        } else {
            oneCount = 0;
        }

        return bit;
    }

    @Override
    public void addGuiComponents(JPanel parent) {
        parent.setLayout(new BorderLayout());

        Utils.addClearContextMenu(packetTable, model::clear);

        JScrollPane scrollPane = new JScrollPane(packetTable,
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        parent.add(scrollPane);
        model.addTableModelListener(tableModelEvent -> {
            scrollPane.setViewportView(packetTable);
            JScrollBar bar = scrollPane.getVerticalScrollBar();
            if (bar != null) bar.setValue(bar.getMaximum());
        });

        var indicatorContainer = new JPanel();
        indicatorContainer.setLayout(new BoxLayout(indicatorContainer, BoxLayout.X_AXIS));
        indicatorContainer.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(2, 2, 2, 2),
                BorderFactory.createEtchedBorder(EtchedBorder.RAISED)));
        indicatorContainer.add(Box.createRigidArea(new Dimension(3, LabeledIndicator.DIMENSIONS.height + 10)));
        indicatorContainer.add(syncIndicator);
        indicatorContainer.add(Box.createHorizontalStrut(8));
        indicatorContainer.add(dataIndicator);
        indicatorContainer.add(Box.createHorizontalGlue());

        parent.add(indicatorContainer, BorderLayout.NORTH);
    }

    private enum State {
        WAIT_FLAG(false, false, "Waiting for flag..."),
        COUNT_FLAG(false, false, "Counting flags..."),
        WAIT_DATA(false, true, "Waiting for begin of data..."),
        WAIT_END(true, true, "Reading data...");

        private final boolean ignoreStuffed;
        private final boolean updateIndicator;
        private final String label;

        State(boolean ignoreStuffed, boolean updateIndicator, String label) {
            this.ignoreStuffed = ignoreStuffed;
            this.updateIndicator = updateIndicator;
            this.label = label;
        }
    }
}
