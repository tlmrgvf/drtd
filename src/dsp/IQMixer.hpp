/*
BSD 2-Clause License

Copyright (c) 2020, Till Mayer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#include <pipe/Component.hpp>
#include <util/Cmplx.hpp>
#include <util/Types.hpp>

namespace Dsp {

class IQMixer final : public RefableComponent<float, Cmplx, IQMixer> {
public:
    IQMixer(Hertz frequency);
    virtual Size calculate_size() override;
    Hertz frequency() const;
    void set_frequency(Hertz);

protected:
    virtual IQMixer& ref() override;
    virtual void draw_at(Point) override;
    virtual Cmplx process(float) override;
    virtual SampleRate on_init(SampleRate, int&) override;
    virtual void show_config_dialog() override;

private:
    static constexpr u8 icon_radius = 11;
    static constexpr Size size { icon_radius * 2 + 1, icon_radius * 2 };

    float m_phase { 0 };
    float m_phase_step { 0 };
    Hertz m_frequency { 0 };
};

}
