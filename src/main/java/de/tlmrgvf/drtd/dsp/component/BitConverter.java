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

package de.tlmrgvf.drtd.dsp.component;

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.utils.Utils;
import de.tlmrgvf.drtd.utils.structure.GenericRingBuffer;

import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.logging.Logger;

public final class BitConverter extends PipelineComponent<Boolean, Boolean> {
    private static BufferedImage IMAGE;
    private static Dimension SIZE;

    static {
        try {
            IMAGE = ImageIO.read(Drtd.readResourceStream("bitconverter.png"));
            SIZE = new Dimension(IMAGE.getWidth(), IMAGE.getHeight());
        } catch (IOException e) {
            Drtd.getLogger(BitConverter.class).throwing("BitConverter", "<clinit>", e);
            Utils.die();
        }
    }

    private final static int BUFFER_SIZE = 1024;
    private final static int MAX_SIMILAR_BITS = 512;
    private final static float SYNC_BIT_ACCURACY = .2F;
    private final static Logger LOGGER = Drtd.getLogger(BitConverter.class);

    private final GenericRingBuffer<Boolean> bitBuffer = new GenericRingBuffer<>(Boolean[]::new, false, BUFFER_SIZE);
    private int requiredSyncBits;
    private float[] baudRates;
    private SyncCallback callback;

    private float[] samplesPerBit;
    private boolean lastSample = false;
    private boolean syncing;
    private int sampleRate;
    private float currentSamplesPerBit = 0;
    private int syncBits = 0;

    private final ReceivedLevel receiving = new ReceivedLevel();
    private final ReceivedLevel previouslyReceived = new ReceivedLevel();

    public synchronized void waitForSync() {
        syncing = true;
        currentSamplesPerBit = 0;
        receiving.clear();
        previouslyReceived.clear();
        bitBuffer.clear(false);
    }

    public BitConverter(float baudRate) {
        super(Boolean.class);
        setBaudRates(baudRate);
    }

    public BitConverter(SyncCallback callback, int requiredSyncBits, float... baudRates) {
        super(Boolean.class);
        setBaudRates(baudRates);
        this.callback = callback;
        this.requiredSyncBits = requiredSyncBits;
    }

    public synchronized void setBaudRates(float... baudRates) {
        if (baudRates.length == 0)
            throw new IllegalArgumentException("No baud rates specified!");

        this.baudRates = baudRates;

        if (baudRates.length == 1) {
            requiredSyncBits = 0;
            syncing = false;
            callback = null;
        } else {
            syncing = true;
        }
        recalculate();
    }

    private void recalculate() {
        samplesPerBit = new float[baudRates.length];
        sampleRate = getInputSampleRate();

        float maxSamples = Float.MAX_VALUE;
        for (int i = 0; i < baudRates.length; ++i) {
            float samples = getInputSampleRate() / baudRates[i];
            samplesPerBit[i] = samples;
            maxSamples = Math.min(samples, maxSamples);
        }

        if (samplesPerBit.length == 1)
            currentSamplesPerBit = samplesPerBit[0];
        else
            currentSamplesPerBit = 0;
    }

    @Override
    public int onInit(int calculatedInputSampleRate) {
        recalculate();
        return calculatedInputSampleRate;
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        return SIZE;
    }

    @Override
    protected void drawRelative(Graphics2D graphics) {
        graphics.drawImage(IMAGE, 0, 0, null);
    }

    @Override
    protected synchronized Boolean calculate(Boolean sample) {
        if (sample == lastSample) {
            ++receiving.samples;
            return bitBuffer.pop();
        }

        receiving.value = lastSample;
        lastSample = sample;

        if (syncing && samplesPerBit.length > 1) {
            if (currentSamplesPerBit == 0) {
                for (float spb : samplesPerBit) {
                    if (Math.abs(1 - ++receiving.samples / spb) <= SYNC_BIT_ACCURACY) {
                        currentSamplesPerBit = spb;
                        ++syncBits;
                        break;
                    }
                }
            } else {
                int cnt = receiving.bitCount();
                if (cnt == 1) {
                    ++syncBits;
                    if (syncBits == requiredSyncBits) {
                        syncBits = 0;
                        syncing = false;

                        float bauds = getBaudrate();
                        LOGGER.fine("Synced to " + bauds + " baud");
                        if (callback != null)
                            callback.onSyncAquired(currentSamplesPerBit, bauds);
                    }
                } else {
                    currentSamplesPerBit = 0;
                    syncBits = 0;
                }
            }

            receiving.clear();
            return bitBuffer.pop();
        }

        if (currentSamplesPerBit == 0)
            return bitBuffer.pop();

        if (bitBuffer.getCount() == BUFFER_SIZE) {
            LOGGER.warning("Buffer full!");
            return bitBuffer.pop();
        }

        int cnt = receiving.bitCount();
        if (cnt == 0) {
            previouslyReceived.samples += receiving.samples;
            receiving.clear();
            return bitBuffer.pop();
        } else if (cnt >= MAX_SIMILAR_BITS) {
            receiving.clear();
            return bitBuffer.pop();
        }

        previouslyReceived.pushIntoBuffer();
        previouslyReceived.from(receiving);
        receiving.clear();

        return bitBuffer.pop();
    }

    public float getBaudrate() {
        return sampleRate / currentSamplesPerBit;
    }

    private class ReceivedLevel {
        private boolean value = false;
        private int samples = 0;

        private void clear() {
            samples = 0;
            value = false;
        }

        private void from(ReceivedLevel l) {
            samples = l.samples;
            value = l.value;
        }

        private int bitCount() {
            return Math.round(samples / currentSamplesPerBit);
        }

        private void pushIntoBuffer() {
            for (int i = 0; i < bitCount(); ++i)
                bitBuffer.push(value);
        }
    }

    public interface SyncCallback {
        void onSyncAquired(float samplesPerBit, float baudRate);
    }
}
