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

import de.tlmrgvf.drtd.dsp.PipelineComponent;
import de.tlmrgvf.drtd.gui.utils.config.ConfigureDialog;
import de.tlmrgvf.drtd.gui.utils.config.FloatSetting;

import java.awt.*;

public final class AcCouple extends PipelineComponent<Float, Float> {
    private float previous;
    private volatile float attenuation;

    public AcCouple(float attenuation) {
        super(Float.class, false);
        this.attenuation = attenuation;
    }

    @Override
    public void showConfigureDialog() {
        ConfigureDialog.showDialog(AcCouple.class,
                "Ac couple",
                new FloatSetting("Factor", this::getAttenuation, this::setAttenuation, 0F, 1F, .001F));
    }

    public float getAttenuation() {
        return attenuation;
    }

    public void setAttenuation(float attenuation) {
        this.attenuation = attenuation;
    }

    @Override
    protected Float calculate(Float input) {
        float w = input + attenuation * previous;
        float ret = w - previous;
        previous = w;
        return ret;
    }

    @Override
    public Dimension calculateSize(Graphics2D g) {
        return new Dimension(6, 14);
    }

    @Override
    protected void drawRelative(Graphics2D graphics) {
        graphics.fillRect(0, 0, 2, 14);
        graphics.fillRect(4, 0, 2, 14);
    }
}
