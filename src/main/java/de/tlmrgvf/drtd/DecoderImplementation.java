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

import de.tlmrgvf.drtd.decoder.*;
import de.tlmrgvf.drtd.decoder.ax25.Ax25;
import de.tlmrgvf.drtd.decoder.pocsag.Pocsag;
import de.tlmrgvf.drtd.utils.Utils;
import org.apache.commons.math3.analysis.function.Log;

import java.lang.reflect.InvocationTargetException;
import java.util.logging.Logger;

public enum DecoderImplementation {
    NULL(Null.class, "Null"),
    AX25(Ax25.class, "AX.25/APRS"),
    RTTY(Rtty.class, "RTTY"),
    POCSAG(Pocsag.class, "POCSAG"),
    DTMF(Dtmf.class, "DTMF"),
    DCF77(Dcf77.class, "DCF77");

    private final Class<? extends Decoder<?>> decoderClass;
    private final String displayName;
    private Decoder<?> instance;

    DecoderImplementation(Class<? extends Decoder<?>> decoderClass, String displayName) {
        this.decoderClass = decoderClass;
        this.displayName = displayName;
    }

    public Decoder<?> getInstance() {
        if (instance == null) {
            try {
                instance = decoderClass.getConstructor().newInstance();
            } catch (InstantiationException |
                    IllegalAccessException |
                    InvocationTargetException |
                    NoSuchMethodException e) {
                /* This has to be done here because the log level will not be set yet if this would be static */
                Logger logger = Drtd.getLogger(DecoderImplementation.class);
                logger.severe("Error while instantiating decoder \"" + this.name() + "\", probably because an " +
                        "exception occurred in the constructor or because no default constructor exists!");
                logger.throwing("DecoderImplementation", "getInstance", e);
                Utils.die();
            }
        }

        return instance;
    }

    public boolean hasHeadlessAvailable() {
        return HeadlessDecoder.class.isAssignableFrom(decoderClass);
    }

    public static DecoderImplementation findByName(String name) {
        name = name.trim();
        for (DecoderImplementation implementation : values()) {
            if (implementation.name().equalsIgnoreCase(name))
                return implementation;
        }

        return null;
    }

    @Override
    public String toString() {
        return displayName;
    }
}
