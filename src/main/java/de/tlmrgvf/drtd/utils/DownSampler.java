package de.tlmrgvf.drtd.utils;

import de.tlmrgvf.drtd.dsp.component.biquad.BiquadFilter;
import de.tlmrgvf.drtd.dsp.component.biquad.GenericBiquadFilter;

public final class DownSampler {
    private final float targetSampleRatio;
    private float currentSampleRatio;
    private float average;
    private float averagedValues;
    private final BiquadFilter lowpassFilter;

    public DownSampler(float sourceSampleRate, float targetSampleRate) {
        targetSampleRatio = sourceSampleRate / targetSampleRate;
        currentSampleRatio = 0;
        lowpassFilter = new BiquadFilter(GenericBiquadFilter.Type.LOWPASS,
                (int) sourceSampleRate,
                targetSampleRate / 2);
    }

    public Float sample(float input) {
        input = lowpassFilter.filterSample(input);
        Float ret = null;

        if (currentSampleRatio >= 0) {
            currentSampleRatio -= targetSampleRatio;
            if (averagedValues == 0)
                return 0F;

            float value = average / averagedValues;
            averagedValues = 0;
            average = 0;
            ret = value;
        }

        ++currentSampleRatio;
        ++averagedValues;
        average += input;
        return ret;
    }
}
