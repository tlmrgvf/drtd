package de.tlmrgvf.drtd.utils;

public final class DownSampler {
    private final float targetSampleRatio;
    private float currentSampleRatio;
    private float average;
    private float averagedValues;

    public DownSampler(float sourceSampleRate, float targetSampleRate) {
        targetSampleRatio = sourceSampleRate / targetSampleRate;
        currentSampleRatio = 0;
    }

    public Float sample(float input) {
        Float ret = null;

        if (currentSampleRatio >= 0) {
            currentSampleRatio -= targetSampleRatio;
            if (averagedValues == 0)
                return 0F;

            var value = average / averagedValues;
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
