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

package de.tlmrgvf.drtd.thread;

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.decoder.Decoder;
import de.tlmrgvf.drtd.utils.Utils;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.TargetDataLine;
import java.io.IOException;
import java.util.logging.Logger;

public final class SoundCardProcessingThread extends ProcessingThread {
    private static final Logger LOGGER = Drtd.getLogger(SoundCardProcessingThread.class);

    private final TargetDataLine line;
    private final AudioInputStream audioInputStream;
    private final byte[] samples = new byte[SAMPLE_BUFFER_SIZE * 2];

    public SoundCardProcessingThread(Decoder<?> decoder, TargetDataLine line) {
        super(decoder);
        this.line = line;
        AudioFormat format = new AudioFormat(decoder.getInputSampleRate(), 16, 1, true, true);
        audioInputStream = new AudioInputStream(line);
        LOGGER.info(String.format(
                "New sound card processing thread with %d channel(s), %d bits, sample rate of %.0f",
                format.getChannels(),
                format.getSampleSizeInBits(),
                format.getSampleRate())
        );

        try {
            line.open(format, SAMPLE_BUFFER_SIZE);
            line.start();
            LOGGER.info(String.format("Actual buffer size: %d", line.getBufferSize()));
        } catch (LineUnavailableException e) {
            LOGGER.throwing("ProcessingThread", "SoundCardProcessingThread", e);
            Utils.die();
        }
    }


    @Override
    public void cleanUp() {
        if (audioInputStream != null) {
            try {
                audioInputStream.close();
            } catch (IOException e) {
                LOGGER.throwing("ProcessingThread", "cleanUp", e);
            }
        }

        line.close();
    }

    @Override
    protected boolean fillBuffer(float[] buffer) {
        try {
            if (audioInputStream.read(samples) != samples.length) {
                LOGGER.warning("Did not read the expected amount of samples, discarding batch!");
                return false;
            }
        } catch (IOException e) {
            LOGGER.throwing("ProcessingThread", "fetchSamples", e);
            return false;
        }

        for (int i = 0; i < buffer.length; ++i) {
            int itt = i * 2;
            buffer[i] = ((short) ((samples[itt] << 8) + (0xFF & samples[itt + 1]))) / (float) Short.MAX_VALUE;
        }

        return true;
    }
}
