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
#include "IQMixerDialog.hpp"
#include <Drtd.hpp>
#include <dsp/IQMixer.hpp>
#include <ui/MainGui.hpp>

using namespace Ui;

static ConfigRef<Dsp::IQMixer> s_current_mixer;

IQMixerDialog::IQMixerDialog()
    : Fl_Window(0, 0, 250, 40, "IQ mixer settings")
    , m_lo_frequency(100, 2, 148, 36, "Frequency:") {
    icon(Drtd::drtd_icon());
    m_lo_frequency.callback(IQMixerDialog::update_mixer);
}

void IQMixerDialog::close_dialog() {
    if (s_iq_mixer_dialog.has_instance()) {
        s_current_mixer = {};
        s_iq_mixer_dialog->hide();
    }
}

void IQMixerDialog::update_mixer(Fl_Widget*) {
    auto& diag = *s_iq_mixer_dialog;
    s_current_mixer->set_frequency(static_cast<Hertz>(diag.m_lo_frequency.value()));
}

void IQMixerDialog::show_dialog(ConfigRef<Dsp::IQMixer> mixer) {
    auto& main_gui = Drtd::main_gui();
    s_iq_mixer_dialog->position(
        main_gui.x() + Util::center(main_gui.w(), s_iq_mixer_dialog->w()),
        main_gui.y() + Util::center(main_gui.h(), s_iq_mixer_dialog->h()));
    s_iq_mixer_dialog->show();
    s_current_mixer = mixer;
    update_dialog();
}

void IQMixerDialog::update_dialog() {
    if (s_iq_mixer_dialog.has_instance() && s_current_mixer.valid()) {
        auto& diag = *s_iq_mixer_dialog;
        diag.m_lo_frequency.range(0, s_current_mixer->input_sample_rate() / 2);
        diag.m_lo_frequency.value(s_current_mixer->frequency());
    }
}
