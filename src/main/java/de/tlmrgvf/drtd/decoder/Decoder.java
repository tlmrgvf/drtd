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
import de.tlmrgvf.drtd.decoder.utils.MarkerGroup;
import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.gui.MainGui;
import de.tlmrgvf.drtd.utils.SettingsManager;

import javax.swing.*;
import java.awt.*;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;

public abstract class Decoder<T> {

    private final int inputSampleRate;
    private final MainGui mainGui = Drtd.getMainGui();
    private final Class<T> resultClass;
    private final SettingsManager manager;

    private PipelineComponent<Float, T> pipeline;
    private PipelineComponent<Float, Float> input;
    private PipelineComponent<T, T> output;
    private MarkerGroup marker;
    private long lastPerformanceDebug = System.currentTimeMillis();
    private long totalComputationDuration = 0;
    private int samplesTaken = 0;
    private final List<T> collectedResults = new LinkedList<>();

    public Decoder(Class<T> resultClass, int inputSampleRate) {
        if (inputSampleRate < 0 || inputSampleRate > 44100)
            throw new IllegalArgumentException("Invalid sample rate");

        this.resultClass = resultClass;
        this.inputSampleRate = inputSampleRate;
        var waterfall = mainGui.getWaterfall();
        manager = SettingsManager.createFor(getClass(), true)
                .mapOption(Float.class, waterfall::getZoom, waterfall::setZoom, 1F)
                .mapOption(Integer.class, waterfall::getFrequencyOffset, waterfall::setFrequencyOffset, 0)
                .mapOption(Boolean.class, waterfall::isZoomOut, waterfall::setZoomOut, false);
        manager.loadAll();
    }

    public final SettingsManager getManager() {
        return manager;
    }

    public abstract void addGuiComponents(JPanel parent);

    protected abstract PipelineComponent<Float, T> buildPipeline();

    protected abstract void onPipelineResult(T result);

    protected abstract void onSetup();

    protected void onMarkerMoved(int centerFrequency) {
    }

    public final PipelineComponent<Float, Float> getInput() {
        return input;
    }

    public final PipelineComponent<T, T> getOutput() {
        return output;
    }

    public final void setup() {
        collectedResults.clear();

        manager.loadAll();
        input = new IoPipelineComponent<>(Float.class);
        input.setMonitoring(true);
        output = new IoPipelineComponent<>(resultClass);
        pipeline = input.pipeline().then(buildPipeline()).build(output);
        pipeline.init(inputSampleRate);
        pipeline.ensureUnique(new HashSet<>(3));

        onSetup();
        setMarker(marker); //Make sure waterfall and main gui are updated
    }

    public final void saveSettings() {
        manager.saveAll();
    }

    protected final void setStatus(String status) {
        mainGui.updateStatus(status);
    }

    public final int getCenterFrequency() {
        return marker == null ? 0 : marker.getCenter();
    }

    public final void setCenterFrequency(int frequency) {
        if (marker == null)
            return;

        frequency = Math.max(-marker.getSmallestFrequency(), frequency);
        marker.setCenter(Math.min(getInputSampleRate(), frequency));
        mainGui.updateFrequencySpinner(getInputSampleRate(), frequency);
        mainGui.getWaterfall().redrawMarker(true);
        onMarkerMoved(frequency);
    }

    public final void process(Float sample) {
        final long start = System.nanoTime();
        final T result = pipeline.calculateGeneric(sample);
        final long diff = System.nanoTime() - start;

        totalComputationDuration += diff;
        ++samplesTaken;

        if (System.currentTimeMillis() - lastPerformanceDebug >= 500) {
            float average = (totalComputationDuration / (float) samplesTaken) / 1000F;
            SwingUtilities.invokeLater(() -> mainGui.getPipelineDialog().updatePerformance(average));
            lastPerformanceDebug = System.currentTimeMillis();
            totalComputationDuration = 0;
            samplesTaken = 0;
        }

        if (result != null)
            synchronized (this) {
                collectedResults.add(result);
            }
    }

    public final void processBatch() {
        synchronized (this) {
            for (T result : collectedResults)
                onPipelineResult(result);

            collectedResults.clear();
        }
    }

    public final PipelineComponent<Float, T> getPipeline() {
        return pipeline;
    }

    public final boolean hasMarker() {
        return marker != null;
    }

    public final MarkerGroup getMarker() {
        return marker;
    }

    public int getInputSampleRate() {
        return inputSampleRate;
    }

    protected final void setMarker(MarkerGroup marker) {
        this.marker = marker;
        mainGui.getWaterfall().enableMarker(marker != null);

        mainGui.enableFrequencySpinner(marker != null && marker.isMoveable());
        if (marker != null)
            setCenterFrequency(marker.getCenter());
    }

    @Override
    public String toString() {
        return "Decoder{" +
                "inputSampleRate=" + inputSampleRate +
                ", resultClass=" + resultClass +
                '}';
    }

    private static class IoPipelineComponent<T> extends PipelineComponent<T, T> {

        private IoPipelineComponent(Class<T> outputClass) {
            super(outputClass);
        }

        @Override
        protected T calculate(T input) {
            return input;
        }

        @Override
        public Dimension calculateSize(Graphics2D g) {
            return new Dimension(10, 10);
        }

        @Override
        public void drawRelative(Graphics2D graphics) {
            graphics.drawRect(0, 0, 9, 9);
            graphics.drawLine(0, 0, 9, 9);
            graphics.drawLine(9, 0, 0, 9);
        }
    }

}
