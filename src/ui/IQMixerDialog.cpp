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
