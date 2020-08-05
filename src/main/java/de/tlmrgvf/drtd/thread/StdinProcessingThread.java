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

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.logging.Logger;

public final class StdinProcessingThread extends ProcessingThread {
    private static final Logger LOGGER = Drtd.getLogger(SoundCardProcessingThread.class);

    private final boolean bigEndian;
    private final SampleSize sampleSize;
    private final BufferedInputStream inputStream = new BufferedInputStream(new InputStream() {
        @Override
        public int read() throws IOException {
            return System.in.read();
        }
    }); //Make sure that System.in will not be closed

    public StdinProcessingThread(Decoder<?> decoder, SampleSize sampleSize, int inputSampleRate, boolean bigEndian) {
        super(decoder, inputSampleRate);
        this.sampleSize = sampleSize;
        this.bigEndian = bigEndian;
    }

    @Override
    public void cleanUp() {
        try {
            inputStream.close();
        } catch (IOException e) {
            LOGGER.throwing("ProcessingThread", "cleanUp", e);
            Utils.die();
        }
    }

    @Override
    protected boolean fillBuffer(float[] buffer) {
        try {
            if (sampleSize == SampleSize.BITS_16) {
                for (int i = 0; i < buffer.length; ++i) {
                    final int first = Math.max(0, inputStream.read());
                    final int second = Math.max(0, inputStream.read());

                    if (bigEndian)
                        buffer[i] = ((short) ((first << 8) + (0xFF & second)))
                                / (float) Short.MAX_VALUE;
                    else
                        buffer[i] = ((short) ((second << 8) + (0xFF & first)))
                                / (float) Short.MAX_VALUE;
                }
            } else {
                for (int i = 0; i < buffer.length; ++i)
                    buffer[i] = Math.max(0, inputStream.read()) / (float) Byte.MAX_VALUE;
            }
        } catch (IOException e) {
            LOGGER.throwing("ProcessingThread", "fillBuffer", e);
            return false;
        }

        return true;
    }

    public enum SampleSize {
        BITS_8,
        BITS_16;

        public static SampleSize getByBits(int bits) {
            switch (bits) {
                case 8:
                    return BITS_8;
                case 16:
                    return BITS_16;
            }
            return null;
        }
    }
}
