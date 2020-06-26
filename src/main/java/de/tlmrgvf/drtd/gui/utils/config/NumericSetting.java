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

package de.tlmrgvf.drtd.gui.utils.config;

import de.tlmrgvf.drtd.utils.Provider;
import de.tlmrgvf.drtd.utils.Setter;

import javax.swing.*;

public abstract class NumericSetting<T extends Number> extends Setting<T> {
    protected final T max;
    protected final T min;
    protected final T step;
    protected JSpinner spinner;

    public NumericSetting(String name, Provider<T> provider, Setter<T> setter, T min, T max, T step) {
        super(name, provider, setter);
        this.min = min;
        this.max = max;
        this.step = step;
        this.spinner = createSpinner();
    }

    public final JComponent createComponent() {
        spinner = createSpinner();
        spinner.addChangeListener((e) -> setSetterOfComponent(spinner.getValue()));
        return spinner;
    }

    protected abstract void setSetterOfComponent(Object value);

    protected abstract JSpinner createSpinner();

    @Override
    public final void loadComponentFromGetter() {
        spinner.setValue(provider.get());
    }
}
