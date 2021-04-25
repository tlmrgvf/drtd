#pragma once

#include <Fl/Fl_Spinner.H>
#include <Fl/Fl_Window.H>
#include <pipe/Component.hpp>
#include <util/Singleton.hpp>

namespace Dsp {
class IQMixer;
}

namespace Ui {

class IQMixerDialog final : Fl_Window {
public:
    static void show_dialog(ConfigRef<Dsp::IQMixer> filter);
    static void close_dialog();
    static void update_dialog();

private:
    static IQMixerDialog* new_dialog() { return new IQMixerDialog(); };
    static inline Singleton<IQMixerDialog, IQMixerDialog::new_dialog> s_iq_mixer_dialog;

    IQMixerDialog();

    static void update_mixer(Fl_Widget*);

    Fl_Spinner m_lo_frequency;
};

}
