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

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.decoder.utils.Marker;
import de.tlmrgvf.drtd.decoder.utils.MarkerGroup;
import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.dsp.component.IQMixer;
import de.tlmrgvf.drtd.dsp.component.Mapper;
import de.tlmrgvf.drtd.dsp.component.Normalizer;
import de.tlmrgvf.drtd.dsp.component.movingaverage.ComplexMovingAverage;
import de.tlmrgvf.drtd.gui.component.LabeledIndicator;
import de.tlmrgvf.drtd.utils.Complex;
import de.tlmrgvf.drtd.utils.Utils;

import javax.swing.*;
import java.awt.*;
import java.util.logging.Logger;

public final class Dcf77 extends HeadlessDecoder<Boolean, String> {
    private final static int SAMPLE_RATE = 6000;
    private final static int BAUDRATE = 10;
    private final static int STATUS_MASK_CALL = 0b10000;
    private final static int STATUS_MASK_CEST = 0b100;
    private final static int STATUS_MASK_CET = 0b10;
    private final static int STATUS_MASK_LEAPSECOND = 0b1;
    private final static int SAMPLES_PER_BIT = SAMPLE_RATE / BAUDRATE;
    private final static Logger LOGGER = Drtd.getLogger(Dcf77.class);
    private final static int[] BCD_LOOKUP = new int[]{1, 2, 4, 8, 10, 20, 40, 80};
    private final static String[] DOW_LOOKUP = new String[]{"---", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

    private final JLabel timeLabel;
    private final JLabel dateLabel;
    private final LabeledIndicator minuteParityIndicator;
    private final LabeledIndicator hourParityIndicator;
    private final LabeledIndicator dateParityIndicator;
    private final LabeledIndicator cestIndicator;
    private final LabeledIndicator cetIndicator;
    private final LabeledIndicator callIndicator;
    private final LabeledIndicator rxIndicator;
    private final LabeledIndicator rxFailIndicator;
    private final IQMixer mixer;
    private State state = State.WAIT_FOR_MINUTE_MARKER;
    private TimeInfo time;
    private TimeInfo receiving;
    private boolean lastLevel = false;
    private boolean parity;
    private boolean tick = false;
    private int levelCount = 0;
    private long lastUpdate = 0;
    private int bitsReveived = 0;
    private int bits = 0;
    private int ticks = 0;
    private int seconds = 0;

    public Dcf77() {
        super(Boolean.class, SAMPLE_RATE);
        timeLabel = new JLabel("--:--:--");
        timeLabel.setFont(Utils.FONT.deriveFont(Font.BOLD, 100F));
        timeLabel.setHorizontalAlignment(SwingConstants.CENTER);

        dateLabel = new JLabel("---, --.--.----");
        dateLabel.setFont(Utils.FONT.deriveFont(Font.BOLD, 35F));
        dateLabel.setHorizontalAlignment(SwingConstants.CENTER);

        minuteParityIndicator = new LabeledIndicator(LabeledIndicator.RED_ON_COLOR, LabeledIndicator.RED_OFF_COLOR,
                "Minute parity");
        hourParityIndicator = new LabeledIndicator(LabeledIndicator.RED_ON_COLOR, LabeledIndicator.RED_OFF_COLOR,
                "Hour parity");
        dateParityIndicator = new LabeledIndicator(LabeledIndicator.RED_ON_COLOR, LabeledIndicator.RED_OFF_COLOR,
                "Date parity");

        cestIndicator = new LabeledIndicator(LabeledIndicator.GREEN_ON_COLOR, LabeledIndicator.GREEN_OFF_COLOR,
                "CEST Time (+2 UTC)");
        cetIndicator = new LabeledIndicator(LabeledIndicator.GREEN_ON_COLOR, LabeledIndicator.GREEN_OFF_COLOR,
                "CET Time (+1 UTC)");
        callIndicator = new LabeledIndicator(LabeledIndicator.RED_ON_COLOR, LabeledIndicator.RED_OFF_COLOR,
                "Abnormal TX operation");

        rxIndicator = new LabeledIndicator(LabeledIndicator.GREEN_ON_COLOR, LabeledIndicator.GREEN_OFF_COLOR,
                "Receiving");
        rxFailIndicator = new LabeledIndicator(LabeledIndicator.RED_ON_COLOR, LabeledIndicator.RED_OFF_COLOR,
                "Last RX failed");

        dateParityIndicator.setAlignmentX(Component.LEFT_ALIGNMENT);
        hourParityIndicator.setAlignmentX(Component.LEFT_ALIGNMENT);
        minuteParityIndicator.setAlignmentX(Component.LEFT_ALIGNMENT);
        cestIndicator.setAlignmentX(Component.LEFT_ALIGNMENT);
        cetIndicator.setAlignmentX(Component.LEFT_ALIGNMENT);
        callIndicator.setAlignmentX(Component.LEFT_ALIGNMENT);
        rxIndicator.setAlignmentX(Component.LEFT_ALIGNMENT);
        rxFailIndicator.setAlignmentX(Component.LEFT_ALIGNMENT);

        mixer = new IQMixer(0);
        setMarker(new MarkerGroup(true, new Marker(0, 10)));

        getSettingsManager().mapOption(Integer.class, this::getCenterFrequency, this::setCenterFrequency, 0);
    }

    @Override
    protected PipelineComponent<Float, Boolean> buildPipeline() {
        return mixer.pipeline()
                .then(new ComplexMovingAverage(SAMPLES_PER_BIT))
                .then(new Mapper<>(Float.class, Complex::magnitude))
                .then(new Normalizer(false, true, (int) (SAMPLE_RATE * 2.2F)))
                .then(new Mapper<>(Boolean.class, f -> f > -.5F))
                .build();
    }

    @Override
    protected void onMarkerMoved(int centerFrequency) {
        mixer.setFrequency(centerFrequency);
    }

    private String createDateString() {
        final var domString = time.dom == 0 ? "--" : String.format("%02d", time.dom);
        final var monthString = time.month == 0 ? "--" : String.format("%02d", time.month);
        final var yearString = time.year < 0 ? "----" : String.format("20%02d", time.year); //:^)
        return String.format("%s, %s.%s.%s", DOW_LOOKUP[time.dow], domString, monthString, yearString);
    }

    private String createTimeString() {
        return String.format("%02d:%02d:%02d", time.hours, time.minutes, seconds);
    }

    private void updateMinute() {
        dateLabel.setText(createDateString());

        minuteParityIndicator.forceState(time.minuteParityError);
        hourParityIndicator.forceState(time.hourParityError);
        dateParityIndicator.forceState(time.dateParityError);
        cestIndicator.forceState(time.cest);
        cetIndicator.forceState(time.cet);
        callIndicator.forceState(time.call);
    }

    private boolean tickTime() {
        if (!tick)
            return false;

        ++ticks;
        if (ticks >= SAMPLE_RATE * 1.1F) {
            advanceTime();
            return true;
        }

        return false;
    }

    private void advanceTime() {
        if (!tick)
            return;

        ticks = 0;
        tick = false;
        ++seconds;
        seconds %= 60;

        timeLabel.setText(createTimeString());
    }

    @Override
    protected void onSetup() {
        state = State.WAIT_FOR_MINUTE_MARKER;
        bitsReveived = 0;
        time = new TimeInfo();
        receiving = new TimeInfo();
        bits = 0;
        timeLabel.setText("--:--:--");
        setStatus(state.description);
        updateMinute();

        minuteParityIndicator.forceState(false);
        hourParityIndicator.forceState(false);
        dateParityIndicator.forceState(false);
        cestIndicator.forceState(false);
        cetIndicator.forceState(false);
        callIndicator.forceState(false);
        rxIndicator.forceState(false);
        rxFailIndicator.forceState(false);
    }

    @Override
    public boolean setupParameters(String[] args) {
        try {
            var center = Integer.parseInt(args[0]);
            if (center < 0 || center > SAMPLE_RATE / 2) {
                printInvalidParameterErrorMessage(0);
                return false;
            }

            setCenterFrequency(center);
            return true;
        } catch (NumberFormatException e) {
            printInvalidParameterErrorMessage(0);
        }

        return false;
    }

    @Override
    public String[] getChangeableParameters() {
        return new String[]{"Center frequency (Int)"};
    }

    @Override
    protected void showResultInGui(String result) {
    }

    @Override
    protected String calculateResult(Boolean result) {
        boolean newSecond = false;

        if (result != lastLevel) {
            int bitCount = Math.round(levelCount / (float) SAMPLES_PER_BIT);
            if (bitCount > 0) {
                levelCount = bitCount;
                setStatus(state.description);

                if (state == State.WAIT_FOR_MINUTE_MARKER) {
                    if (lastLevel && levelCount > BAUDRATE * 1.5 && levelCount <= 2 * BAUDRATE) {
                        state = State.READ_START_OF_MINUTE;
                        bitsReveived = 0;
                        bits = 0;
                        seconds = -1;
                        time = receiving;
                        receiving = new TimeInfo();
                        updateMinute();
                        tick = true;
                        LOGGER.fine("Detected minute marker");
                    }
                } else {
                    if (!lastLevel) {
                        bits <<= 1;

                        if (levelCount == 2) {
                            bits |= 1;
                            parity = !parity;
                        } else if (levelCount != 1) {
                            LOGGER.warning("Invalid bit received!");
                            state = State.WAIT_FOR_MINUTE_MARKER;
                            rxFailIndicator.forceState(true);
                            receiving = new TimeInfo();
                        }

                        if (state.pointOfRead == bitsReveived) {
                            LOGGER.fine("Received in " + state + ": " + Integer.toBinaryString(bits));
                            if (state == State.READ_START_OF_MINUTE && bits == 1 ||
                                    state == State.READ_START_OF_TIME && bits == 0) {
                                LOGGER.warning("Did not receive expected marker bit!");
                                state = State.WAIT_FOR_MINUTE_MARKER;
                                receiving = new TimeInfo();
                                rxFailIndicator.forceState(true);
                            } else {
                                switch (state) {
                                    case READ_STATUS:
                                        receiving.call = (bits & STATUS_MASK_CALL) != 0;
                                        receiving.cet = (bits & STATUS_MASK_CET) != 0;
                                        receiving.cest = (bits & STATUS_MASK_CEST) != 0;

                                        if ((bits & STATUS_MASK_LEAPSECOND) != 0)
                                            onSetup(); //When receiving a leap second announcement, just reset
                                        break;
                                    case READ_MINUTES:
                                        receiving.minuteParityError = parity;
                                        receiving.minutes = decodeBcd(bits >> 1, 7);
                                        break;
                                    case READ_HOURS:
                                        receiving.hourParityError = parity;
                                        receiving.hours = decodeBcd(bits >> 1, 6);
                                        break;
                                    case READ_DAY_OF_MONTH:
                                        receiving.dom = decodeBcd(bits, 6);
                                        break;
                                    case READ_DAY_OF_WEEK:
                                        receiving.dow = decodeBcd(bits, 3);
                                        break;
                                    case READ_MONTH_NUMBER:
                                        receiving.month = decodeBcd(bits, 5);
                                        break;
                                    case READ_YEAR:
                                        receiving.year = decodeBcd(bits, 8);
                                        break;
                                    case READ_DATE_PARITY:
                                        receiving.dateParityError = parity;
                                        break;
                                }

                                state = State.nextState(state);
                                setStatus(state.description);
                            }

                            bits = 0;
                            if (!state.dateState)
                                parity = false;
                        }

                        bitsReveived++;
                        if (bitsReveived == 59) {
                            LOGGER.fine("Successfully received!");
                            rxFailIndicator.forceState(false);
                            state = State.WAIT_FOR_MINUTE_MARKER;
                        }

                        rxIndicator.forceState(state != State.WAIT_FOR_MINUTE_MARKER);
                    }
                }

                if (!lastLevel) {
                    /* Sanity check */
                    if (System.currentTimeMillis() - lastUpdate > 500) {
                        lastUpdate = System.currentTimeMillis();
                        advanceTime();
                        tick = true;
                        newSecond = true;
                    }
                }

                levelCount = 0;
            }

            lastLevel = result;
        }

        ++levelCount;
        if (tickTime() || newSecond) {
            boolean timeValid = (time.cest & !time.cet) || (time.cet & !time.cest);
            return String.format("%s - %s ; %s %s %s\n",
                    createDateString(),
                    createTimeString(),
                    ((time.hourParityError ||
                            time.minuteParityError ||
                            time.dateParityError ||
                            !timeValid) ? "E" : ""),
                    (time.cest ? "CEST" : ""),
                    (time.cet ? "CET" : "")
            );
        }

        return null;
    }

    @Override
    public void addGuiComponents(JPanel parent) {
        parent.setLayout(new BoxLayout(parent, BoxLayout.Y_AXIS));
        Utils.addLabeledComponent(parent, dateLabel, "Date");
        Utils.addLabeledComponent(parent, timeLabel, "Time");

        var statusPanel = new JPanel();
        statusPanel.setLayout(new BoxLayout(statusPanel, BoxLayout.X_AXIS));
        parent.add(statusPanel);

        statusPanel.add(Box.createHorizontalGlue());

        var errorPanel = new JPanel();
        errorPanel.setLayout(new BoxLayout(errorPanel, BoxLayout.Y_AXIS));
        errorPanel.setBorder(Utils.createLabeledBorder("Errors"));
        errorPanel.add(minuteParityIndicator);
        errorPanel.add(Box.createRigidArea(new Dimension(1, 4)));
        errorPanel.add(hourParityIndicator);
        errorPanel.add(Box.createRigidArea(new Dimension(1, 4)));
        errorPanel.add(dateParityIndicator);
        statusPanel.add(errorPanel);
        statusPanel.add(Box.createHorizontalGlue());

        var flagPanel = new JPanel();
        flagPanel.setLayout(new BoxLayout(flagPanel, BoxLayout.Y_AXIS));
        flagPanel.setBorder(Utils.createLabeledBorder("Flags"));
        flagPanel.add(cetIndicator);
        flagPanel.add(Box.createRigidArea(new Dimension(1, 4)));
        flagPanel.add(cestIndicator);
        flagPanel.add(Box.createRigidArea(new Dimension(1, 4)));
        flagPanel.add(callIndicator);
        statusPanel.add(flagPanel);
        statusPanel.add(Box.createHorizontalGlue());

        var miscPanel = new JPanel();
        miscPanel.setLayout(new BoxLayout(miscPanel, BoxLayout.Y_AXIS));
        miscPanel.setBorder(Utils.createLabeledBorder("Miscellaneous"));
        miscPanel.add(rxIndicator);
        miscPanel.add(Box.createRigidArea(new Dimension(1, 4)));
        miscPanel.add(rxFailIndicator);
        statusPanel.add(miscPanel);
        statusPanel.add(Box.createHorizontalGlue());
    }

    private static int decodeBcd(int bits, int bitCount) {
        int result = 0;
        for (int i = 0; i < bitCount; ++i) {
            if ((bits & 1) != 0)
                result += BCD_LOOKUP[bitCount - i - 1];
            bits >>= 1;
        }

        return result;
    }

    private static final class TimeInfo {
        private int minutes = 0;
        private int hours = 0;
        private int dow = 0;
        private int dom = 0;
        private int year = -1;
        private int month = 0;
        private boolean call;
        private boolean cet;
        private boolean cest;
        private boolean hourParityError;
        private boolean minuteParityError;
        private boolean dateParityError;
    }

    private enum State {
        WAIT_FOR_MINUTE_MARKER(0, "Waiting for begin of minute..."),
        READ_START_OF_MINUTE(0, "Reading start of minute..."),
        READ_CIVIL_WARINING(14, "Reading civil warning bits..."),
        READ_STATUS(19, "Reading status bits..."),
        READ_START_OF_TIME(20, "Reading start of time bit..."),
        READ_MINUTES(28, "Reading minute..."),
        READ_HOURS(35, "Reading hour..."),
        READ_DAY_OF_MONTH(41, "Reading day of month...", true),
        READ_DAY_OF_WEEK(44, "Reading day of week...", true),
        READ_MONTH_NUMBER(49, "Reading month...", true),
        READ_YEAR(57, "Reading year...", true),
        READ_DATE_PARITY(58, "Reading date parity...", true);

        private final int pointOfRead;
        private final String description;
        private final boolean dateState;

        State(int pointOfRead, String description, boolean dateState) {
            this.pointOfRead = pointOfRead;
            this.description = description;
            this.dateState = dateState;
        }

        State(int pointOfRead, String description) {
            this(pointOfRead, description, false);
        }

        private static State nextState(State state) {
            var values = values();
            for (int i = 0; i < values.length; ++i) {
                if (values[i] == state) {
                    return values[(i + 1) % values.length];
                }
            }
            assert false;
            return null;
        }
    }
}
