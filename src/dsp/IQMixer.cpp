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
#include "IQMixer.hpp"
#include <Fl/fl_draw.H>
#include <cmath>
#include <ui/IQMixerDialog.hpp>
#include <util/Cmplx.hpp>

using namespace Dsp;

IQMixer::IQMixer(Hertz frequency)
    : RefableComponent<float, Cmplx, IQMixer>("IQ Mixer")
    , m_frequency(frequency) {
}

Size IQMixer::calculate_size() {
    return size;
}

IQMixer& Dsp::IQMixer::ref() {
    return *this;
}

Hertz IQMixer::frequency() const {
    return m_frequency;
}

void IQMixer::set_frequency(Hertz frequency) {
    if (frequency == m_frequency)
        return;

    m_frequency = frequency;
    m_phase_step = Util::two_pi_f / input_sample_rate() * static_cast<float>(m_frequency);
    Ui::IQMixerDialog::update_dialog();
}

SampleRate IQMixer::on_init(SampleRate input_sample_rate, int&) {
    m_phase_step = Util::two_pi_f / input_sample_rate * static_cast<float>(m_frequency);
    return input_sample_rate;
}

void IQMixer::show_config_dialog() {
    Ui::IQMixerDialog::show_dialog(make_ref());
}

void IQMixer::draw_at(Point p) {
    fl_circle(p.x() + size.w() / 2, p.y() + size.h() / 2, icon_radius);
    const unsigned corner_from_center = static_cast<unsigned>(std::roundf(std::sqrt(icon_radius * icon_radius / 2.f)));

    p.translate(icon_radius, icon_radius);
    fl_line(p.x(), p.y(), p.x() + corner_from_center, p.y() + corner_from_center);
    fl_line(p.x(), p.y(), p.x() + corner_from_center, p.y() - corner_from_center);
    fl_line(p.x(), p.y(), p.x() - corner_from_center, p.y() + corner_from_center);
    fl_line(p.x(), p.y(), p.x() - corner_from_center, p.y() - corner_from_center);
}

Cmplx IQMixer::process(float sample) {
    Cmplx result(sample * std::cos(m_phase), sample * -std::sin(m_phase));
    m_phase += m_phase_step;
    m_phase = std::remainder(m_phase, Util::two_pi_f);
    return result;
}
