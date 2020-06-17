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

import de.tlmrgvf.drtd.dsp.Interpreter;
import de.tlmrgvf.drtd.dsp.SimpleInterpreter;
import de.tlmrgvf.drtd.utils.Complex;

import java.awt.*;

public enum ValueInterpreter {
    FLOAT(Float.class, new SimpleInterpreter("Float", new Color(.9F, .6F, .2F)) {
        @Override
        public Float interpret(Object value) {
            return (Float) value;
        }
    }),
    INTEGER(Integer.class, new SimpleInterpreter("Integer", new Color(.4F, .8F, .4F)) {
        @Override
        public Float interpret(Object value) {
            return (float) (int) value; //Can't cast a Integer directly to a Float
        }
    }),
    BOOLEAN(Boolean.class, new SimpleInterpreter("Boolean", new Color(.9F, .3F, .3F)) {

        @Override
        public Float interpret(Object value) {
            return ((boolean) value) ? 1F : 0F;
        }
    }),
    COMPLEX(Complex.class, Complex.createInterpreter());

    private final Interpreter instance;
    private final Class<?> classType;

    ValueInterpreter(Class<?> classType, Interpreter instance) {
        this.instance = instance;
        this.classType = classType;
    }

    public Interpreter getInstance() {
        return instance;
    }

    public Class<?> getClassType() {
        return classType;
    }
}
