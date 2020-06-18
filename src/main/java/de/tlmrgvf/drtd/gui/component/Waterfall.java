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

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package de.tlmrgvf.drtd.gui.component;

import de.tlmrgvf.drtd.Drtd;
import de.tlmrgvf.drtd.decoder.Decoder;
import de.tlmrgvf.drtd.decoder.utils.Marker;
import de.tlmrgvf.drtd.dsp.window.RectangularWindow;
import de.tlmrgvf.drtd.dsp.window.Window;
import de.tlmrgvf.drtd.gui.dialog.WaterfallDialog;
import de.tlmrgvf.drtd.gui.utils.Canvas;
import de.tlmrgvf.drtd.gui.utils.Layer;
import de.tlmrgvf.drtd.utils.DownSampler;
import de.tlmrgvf.drtd.utils.SettingsManager;
import de.tlmrgvf.drtd.utils.Utils;
import de.tlmrgvf.drtd.utils.structure.RingBuffer;
import org.jtransforms.fft.FloatFFT_1D;

import java.awt.*;
import java.awt.event.*;
import java.util.logging.Logger;

public final class Waterfall extends Canvas {
    private final static SettingsManager SETTINGS_MANAGER = SettingsManager.createFor(Waterfall.class);
    public final static float MAX_CONTRAST = .001F;
    public final static float MIN_CONTRAST = 1F;

    private final static int MARKER_AREA_HEIGHT = 30;
    private final static int MARKER_MAJOR_HEIGHT = 10;
    private final static int MARKER_MINOR_HEIGHT = 5;

    private final Layer waterfallLayer;
    private final Layer markerLayer;
    private final Layer scaleLayer;
    private final Font markerFont;
    private final RingBuffer sampleBuffer;

    private Window windowProvider;
    private WaterfallPalette palette;
    private float[] windowCoefficients;
    private FloatFFT_1D fft;
    private int sampleCount;
    private int sampleRate;
    private float hertzPerBin;
    private Decoder<?> decoder;

    private int frequencyOffset = 0;
    private int binOffset = 0;
    private float contrast = MIN_CONTRAST;
    private float zoom = 1F;
    private int bins;
    private int speedMultiplier = 1;
    private boolean powerSpectrum = false;
    private boolean showMarker = false;
    private boolean zoomOut = false;

    public Waterfall(int bins) {
        assert bins > 0 && bins % 2 == 0;

        this.bins = bins;
        waterfallLayer = createLayer(0, MARKER_AREA_HEIGHT, Layer.PARENT_SIZE, Layer.PARENT_SIZE);
        scaleLayer = createLayer(0, 0, Layer.PARENT_SIZE, MARKER_AREA_HEIGHT);
        markerLayer = createLayer(0, 0, 1, Layer.PARENT_SIZE);

        ListenerImpl listener = new ListenerImpl();
        setResizeListener(listener);
        addMouseListener(listener);
        addMouseMotionListener(listener);
        addMouseWheelListener(listener);

        sampleBuffer = new RingBuffer(bins);
        fft = new FloatFFT_1D(bins);
        windowProvider = new RectangularWindow();
        windowCoefficients = windowProvider.calculateCoefficients(bins);
        markerFont = Utils.FONT.deriveFont(14F);

        SETTINGS_MANAGER
                .mapOption(Float.class, this::getContrast, this::setContrast, MIN_CONTRAST)
                .mapOption(Integer.class, this::getBins, this::setBins, 4096)
                .mapOption(Window.class, this::getWindow, this::setWindow, new RectangularWindow())
                .mapOption(Integer.class, this::getSpeedMultiplier, this::setSpeedMultiplier, 1)
                .mapOption(Boolean.class, this::isPowerSpectrum, this::setPowerSpectrum, false)
                .mapOption(WaterfallPalette.class, this::getPalette, this::setPalette, WaterfallPalette.COLORFUL)
                .loadAll();

        recomputeParameters();
    }

    public synchronized void process(Float sample) {
        sampleBuffer.push(sample);

        if (sampleCount < bins / speedMultiplier) {
            ++sampleCount;
            return;
        }

        sampleCount = 0;
        final Graphics2D waterfallGraphics = waterfallLayer.getGraphics();

        final float[] unwrapped = new float[bins * 2];
        final Float[] bufferContents = sampleBuffer.getContents();
        final int firstElement = sampleBuffer.getNextEmptyPosition();

        for (int i = 0; i < bufferContents.length; ++i) {
            unwrapped[i * 2] = bufferContents[(i + firstElement) % sampleBuffer.getSize()] * windowCoefficients[i];
            unwrapped[i * 2 + 1] = 0;
        }

        fft.complexForward(unwrapped);

        final DownSampler downsampler = new DownSampler(1, binZoomFactor());
        float max = 0;
        int downsampleIndex = 0;
        final int pseudoBins = Math.round(binZoomFactor() * bins);
        final float[] binValues = new float[zoomOut ? pseudoBins / 2 : bins / 2];

        for (int i = 0; i < unwrapped.length / 2; i += 2) {
            Float magnitude = unwrapped[i] * unwrapped[i] + unwrapped[i + 1] * unwrapped[i + 1];
            if (!powerSpectrum)
                magnitude = (float) Math.sqrt(magnitude);

            if (zoomOut) {
                if ((magnitude = downsampler.sample(magnitude)) != null) {
                    max = Math.max(max, magnitude);
                    if (downsampleIndex >= binValues.length)
                        break;

                    binValues[downsampleIndex++] = magnitude;
                }
            } else {
                binValues[i / 2] = magnitude;
                max = Math.max(max, magnitude);
            }
        }

        waterfallGraphics.copyArea(0, 0, getWidth(), getHeight() - MARKER_AREA_HEIGHT, 0, 1);
        final Color[] paletteColors = palette.getColors();
        int x = 0;
        for (int i = binOffset; i < pseudoBins / 2 && x < getWidth(); ++i, ++x) {
            float value = zoomOut ? binValues[i] :
                    Utils.linearInterpolate(i / (float) pseudoBins * bins, binValues);
            if (Float.isNaN(value))
                value = 0;

            final float floatIndex = (float) Utils.scaleLog(
                    max == 0 ? 0 : value / max,
                    MIN_CONTRAST - contrast,
                    1,
                    0,
                    paletteColors.length - 1,
                    false
            );

            final int baseIndex = (int) floatIndex;
            final Color baseColor = paletteColors[baseIndex];
            final Color mixColor = paletteColors[Math.min(paletteColors.length - 1, baseIndex + 1)];
            final float mixAmount = floatIndex - baseIndex;
            final float baseAmount = 1 - mixAmount;

            waterfallGraphics.setColor(new Color(
                    (int) ((mixAmount * mixColor.getRed()) + (baseAmount * baseColor.getRed())),
                    (int) ((mixAmount * mixColor.getGreen()) + (baseAmount * baseColor.getGreen())),
                    (int) ((mixAmount * mixColor.getBlue()) + (baseAmount * baseColor.getBlue()))
            ));
            waterfallGraphics.drawRect(x, 0, 0, 0);
        }

        waterfallGraphics.setColor(Color.BLACK);
        waterfallGraphics.drawLine(x, 0, getWidth(), 0);

        drawLayers(false);
    }

    private synchronized void redrawScale() {
        final Graphics2D scaleGraphics = scaleLayer.getGraphics();
        int lastFrequency = translateFrequency(-1) / 500;

        scaleGraphics.setFont(markerFont);
        scaleGraphics.setColor(Color.BLACK);
        scaleGraphics.fillRect(0, 0, getWidth(), MARKER_AREA_HEIGHT);
        scaleGraphics.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);

        for (int x = 0; x < bins / 2 && x < getWidth(); ++x) {
            final int frequencyAtX = translateFrequency(x);
            final int frequencyStepped = frequencyAtX / 500;

            if (frequencyStepped != lastFrequency) {
                int markerTop = MARKER_AREA_HEIGHT - (frequencyStepped % 2 == 0 ? MARKER_MAJOR_HEIGHT : MARKER_MINOR_HEIGHT);

                scaleGraphics.setColor(Color.WHITE);
                scaleGraphics.drawLine(x, markerTop, x, MARKER_AREA_HEIGHT);

                if (frequencyStepped % 2 == 0) {
                    String txt = (frequencyStepped / 2) + "";

                    int centerX = (int) markerFont.getStringBounds(txt, scaleGraphics.getFontRenderContext()).getCenterX();
                    scaleGraphics.drawString(txt, x - centerX, markerTop - 2);
                }
                lastFrequency = frequencyStepped;
            }

            if (frequencyAtX > sampleRate / 2)
                break;
        }

        drawLayers(false);
    }

    public synchronized void redrawMarker(boolean forceDraw) {
        if (decoder == null || decoder.getPipeline() == null)
            return;

        var marker = decoder.getMarker();
        if (marker == null || !showMarker || !decoder.getInput().isBeingMonitored()) {
            markerLayer.setWidth(0);
            drawLayers(false);
            return;
        }

        int halfWidth = 0;
        for (Marker mark : marker.getMarkers())
            halfWidth = translateXWithoutOffset(Math.max(halfWidth,
                    Math.abs(mark.getOffset() + mark.getBandwidth() / 2)));

        if (halfWidth == 0) return;
        markerLayer.setWidth(2 * halfWidth);

        markerLayer.clear();
        Graphics markerGraphics = markerLayer.getGraphics();

        markerGraphics.setColor(Color.YELLOW);
        markerGraphics.drawLine(halfWidth, 0, halfWidth, markerLayer.getHeight());
        for (Marker mark : marker.getMarkers()) {
            final int bwOffset = translateXWithoutOffset(mark.getBandwidth());
            final int x = halfWidth + translateXWithoutOffset(mark.getOffset());
            final int bwx = x - bwOffset / 2;

            markerGraphics.setColor(Color.RED);
            markerGraphics.drawLine(x, 0, x, markerLayer.getHeight());
            markerGraphics.setColor(new Color(1, 0, 0, 0.3F));
            markerGraphics.fillRect(bwx, 0, bwOffset, markerLayer.getHeight());
        }

        moveMarker(translateX(marker.getCenter()), false);
        drawLayers(forceDraw);
    }

    public WaterfallPalette getPalette() {
        return palette;
    }

    public synchronized void setPalette(WaterfallPalette palette) {
        this.palette = palette;
    }

    public boolean isZoomOut() {
        return zoomOut;
    }

    public synchronized void setZoomOut(boolean zoomOut) {
        this.zoomOut = zoomOut;
        recomputeParameters();
        redrawMarker(true);
        redrawScale();
    }

    public synchronized void enableMarker(boolean showMarker) {
        this.showMarker = showMarker;
        redrawMarker(true);
    }

    public boolean isPowerSpectrum() {
        return powerSpectrum;
    }

    public synchronized void setPowerSpectrum(boolean powerSpectrum) {
        this.powerSpectrum = powerSpectrum;
    }

    public int getSampleRate() {
        return sampleRate;
    }

    public synchronized void setSampleRate(int sampleRate) {
        this.sampleRate = sampleRate;
        recomputeParameters();
    }

    private void recomputeParameters() {
        hertzPerBin = sampleRate / (float) bins / binZoomFactor();

        setFrequencyOffset(frequencyOffset); //Recalculate bin offset
    }

    public int getSpeedMultiplier() {
        return speedMultiplier;
    }

    public synchronized void setSpeedMultiplier(int speedMultiplier) {
        this.speedMultiplier = speedMultiplier;
    }

    private float binZoomFactor() {
        return zoomOut ? 1 / zoom : zoom;
    }

    public Float getZoom() {
        return zoom;
    }

    public synchronized void setZoom(float zoom) {
        this.zoom = zoom;
        recomputeParameters();
        redrawMarker(true);
        redrawScale();
    }

    public int getBins() {
        return bins;
    }

    public synchronized void setBins(int bins) {
        this.bins = bins;
        windowCoefficients = windowProvider.calculateCoefficients(bins);
        sampleBuffer.resize(bins);
        fft = new FloatFFT_1D(bins);
        recomputeParameters();
    }

    public float getContrast() {
        return contrast;
    }

    public synchronized void setContrast(float contrast) {
        this.contrast = Utils.clampBetween(MAX_CONTRAST, MIN_CONTRAST, contrast);
    }

    public synchronized void setDecoder(Decoder<?> decoder) {
        this.decoder = decoder;
        setSampleRate(decoder.getInputSampleRate());
        enableMarker(true);
        redrawMarker(true);
    }

    public Window getWindow() {
        return windowProvider;
    }

    public synchronized void setWindow(Window windowProvider) {
        this.windowProvider = windowProvider;
        this.windowCoefficients = windowProvider.calculateCoefficients(bins);
    }

    public int getFrequencyOffset() {
        return frequencyOffset;
    }

    public synchronized void setFrequencyOffset(int frequencyOffset) {
        this.frequencyOffset = Utils.clampBetween(0, sampleRate / 2, frequencyOffset);
        this.binOffset = (int) (this.frequencyOffset / hertzPerBin);
        redrawScale();

        if (decoder != null && decoder.hasMarker())
            moveMarker(translateX(decoder.getMarker().getCenter()), false);
    }

    private int translateFrequency(int xOnCanvas) {
        return Math.round((binOffset + xOnCanvas) * hertzPerBin);
    }

    private int translateX(int frequency) {
        return translateXWithoutOffset(frequency - binOffset * hertzPerBin);
    }

    private int translateXWithoutOffset(float frequency) {
        return Math.round(frequency / hertzPerBin);
    }

    public synchronized void moveMarkerToFrequency(int hz) {
        var marker = decoder.getMarker();
        if (decoder.hasMarker() && marker.isMoveable())
            moveMarker(Math.min(translateX(hz), translateX(sampleRate / 2)), true);
    }

    private void updateMarkerFromWaterfall(int mouseX) {
        if (decoder.hasMarker() &&
                !decoder.getMarker().isMoveable() || !decoder.getInput().isBeingMonitored())
            return;

        int frequency = translateFrequency(mouseX);
        moveMarkerToFrequency(frequency);
        decoder.setCenterFrequency(frequency);
    }

    private void moveMarker(int xPos, boolean mouse) {
        int halfWidth = markerLayer.getWidth() / 2;
        int xCenter = mouse ? Math.max(0, xPos - halfWidth) : xPos - halfWidth;

        markerLayer.setX(xCenter);
        drawLayers(false);
    }

    private class ListenerImpl extends MouseAdapter implements Canvas.CanvasResizeListener, MouseMotionListener,
            MouseWheelListener {
        private final Logger LOGGER = Drtd.getLogger(getClass());

        @Override
        public void onCanvasResized(Canvas c) {
            redrawMarker(false);
            redrawScale();
        }

        @Override
        public void mouseClicked(MouseEvent mouseEvent) {
            LOGGER.info("Clicked at " + translateFrequency(mouseEvent.getX()) + " Hz");

            if (mouseEvent.getButton() == MouseEvent.BUTTON3) {
                var dialog = WaterfallDialog.getInstance(Waterfall.this);
                dialog.setVisible(true);
                dialog.updateUiFromWaterfall();
                dialog.setState(Frame.NORMAL);
            }
        }

        @Override
        public void mousePressed(MouseEvent mouseEvent) {
            if ((mouseEvent.getModifiersEx() & MouseEvent.BUTTON1_DOWN_MASK) != 0)
                updateMarkerFromWaterfall(mouseEvent.getX());
        }

        @Override
        public void mouseReleased(MouseEvent mouseEvent) {
            if ((mouseEvent.getModifiersEx() & MouseEvent.BUTTON1_DOWN_MASK) != 0)
                updateMarkerFromWaterfall(mouseEvent.getX());
        }

        @Override
        public void mouseWheelMoved(MouseWheelEvent event) {
            int wheelRotation = event.getWheelRotation();

            if ((event.getModifiersEx() & MouseEvent.SHIFT_DOWN_MASK) == 0) {
                setFrequencyOffset(getFrequencyOffset() - wheelRotation * 250);
            } else {
                final var zoom = getZoom() + (wheelRotation / 10F) * (zoomOut ? 2 : -2);
                if (zoom < 1) {
                    zoomOut = !zoomOut;
                    setZoom(1.1F);
                } else {
                    setZoom(zoom);
                }
            }

            WaterfallDialog.getInstance(Waterfall.this).updateUiFromWaterfall();
        }

        @Override
        public void mouseDragged(MouseEvent mouseEvent) {
            if ((mouseEvent.getModifiersEx() & MouseEvent.BUTTON1_DOWN_MASK) != 0)
                updateMarkerFromWaterfall(mouseEvent.getX());
        }
    }

}
