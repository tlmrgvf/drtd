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

package de.tlmrgvf.drtd;

import de.tlmrgvf.drtd.decoder.Decoder;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.TargetDataLine;
import java.io.IOException;
import java.util.logging.Logger;

public final class ProcessingThread extends Thread {
    private final static int AUDIO_BUFFER_SIZE = 1024;
    private static final Logger LOGGER = Drtd.getLogger(ProcessingThread.class);

    private final Decoder<?> decoder;
    private final int sampleRate;
    private final TargetDataLine line;
    private volatile boolean run = true;

    public ProcessingThread(TargetDataLine line, Decoder<?> decoder, int sampleRate) {
        super("ProcessingThread");
        setDaemon(true);
        this.line = line;
        this.decoder = decoder;
        this.sampleRate = sampleRate;
    }

    public Decoder<?> getDecoder() {
        return decoder;
    }

    public void requestStop() {
        run = false;
    }

    @Override
    public void run() {
        final var format = new AudioFormat(sampleRate, 16, 1, true, true);
        LOGGER.info(String.format(
                "Started processing thread with %d channel(s), %d bits, sample rate of %.0f",
                format.getChannels(),
                format.getSampleSizeInBits(),
                format.getSampleRate())
        );

        try (line) {
            try (var inputStream = new AudioInputStream(line)) {
                line.open(format, AUDIO_BUFFER_SIZE);
                line.start();
                LOGGER.info(String.format("Actual buffer size: %d", line.getBufferSize()));

                short[] processed = new short[AUDIO_BUFFER_SIZE / 2];
                byte[] samples = new byte[AUDIO_BUFFER_SIZE];

                while (run) {
                    if (inputStream.read(samples) != samples.length) {
                        LOGGER.warning("Did not read the expected amount of samples, discarding batch!");
                        continue;
                    }

                    for (int i = 0; i < processed.length; ++i) {
                        int itt = i * 2;
                        processed[i] = (short) ((samples[itt] << 8) + (0xFF & samples[itt + 1]));
                    }

                    if (run) {
                        for (short i : processed)
                            decoder.process((i / (float) Short.MAX_VALUE));
                    }
                }
            } catch (IOException e) {
                LOGGER.throwing("ProcessingThread", "run", e);
            }
        } catch (LineUnavailableException e) {
            LOGGER.throwing("ProcessingThread", "run", e);
        }

        LOGGER.info("Processing thread stopped!");
    }
}
