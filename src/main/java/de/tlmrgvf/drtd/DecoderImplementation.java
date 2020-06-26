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

import de.tlmrgvf.drtd.decoder.Decoder;
import de.tlmrgvf.drtd.decoder.Dtmf;
import de.tlmrgvf.drtd.decoder.Null;
import de.tlmrgvf.drtd.decoder.Rtty;
import de.tlmrgvf.drtd.decoder.ax25.Ax25;
import de.tlmrgvf.drtd.decoder.pocsag.Pocsag;
import de.tlmrgvf.drtd.utils.Provider;

public enum DecoderImplementation {
    NULL(Null::new, "Null"),
    AX25(Ax25::new, "AX.25/APRS"),
    RTTY(Rtty::new, "RTTY"),
    POCSAG(Pocsag::new, "POCSAG"),
    DTMF(Dtmf::new, "DTMF");

    private final Provider<Decoder<?>> constructor;
    private final String displayName;
    private Decoder<?> instance;

    DecoderImplementation(Provider<Decoder<?>> constructor, String displayName) {
        this.constructor = constructor;
        this.displayName = displayName;
    }

    public Decoder<?> getInstance() {
        instance = instance == null ? constructor.get() : instance;
        return instance;
    }

    @Override
    public String toString() {
        return displayName;
    }
}
