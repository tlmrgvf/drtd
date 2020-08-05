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

package de.tlmrgvf.drtd.utils;

import de.tlmrgvf.drtd.dsp.component.biquad.BiquadFilter;
import de.tlmrgvf.drtd.dsp.component.biquad.GenericBiquadFilter;

public final class ReSampler {
    private final float targetSampleRatio;
    private final BiquadFilter lowPassFilter;
    private final boolean downSample;
    private float currentSampleRatio;
    private float average;
    private float averagedValues;
    private float currentSample;

    public ReSampler(float sourceSampleRate, float targetSampleRate) {
        targetSampleRatio = sourceSampleRate / targetSampleRate;
        currentSampleRatio = 0;
        if (sourceSampleRate > targetSampleRate) {
            lowPassFilter = new BiquadFilter(GenericBiquadFilter.Type.LOWPASS,
                    (int) sourceSampleRate,
                    targetSampleRate / 2);
        } else {
            lowPassFilter = null;
        }

        downSample = lowPassFilter != null;
    }

    public void processInputSample(float sample) {
        if (downSample) {
            sample = lowPassFilter.filterSample(sample);
            average += sample;
            ++averagedValues;
        }

        currentSample = sample;
        ++currentSampleRatio;
    }

    public Float readOutputSample() {
        if (currentSampleRatio >= 0) {
            currentSampleRatio -= targetSampleRatio;

            if (downSample) {
                if (averagedValues == 0)
                    return 0F;

                float value = average / averagedValues;
                averagedValues = 0;
                average = 0;
                return value;
            } else {
                return currentSample;
            }
        }

        return null;
    }
}
