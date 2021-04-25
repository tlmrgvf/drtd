#pragma once

#include <Fl/Fl_Check_Button.H>
#include <Fl/Fl_Light_Button.H>
#include <Fl/Fl_Slider.H>
#include <Fl/Fl_Window.H>
#include <stdio.h>
#include <util/Singleton.hpp>

namespace Ui {

class ScopeDialog final : Fl_Window {
public:
    static void show_dialog();
    static void load_from_scope();
    static void save_to_scope(Fl_Widget*, void*);

private:
    ScopeDialog();

    static ScopeDialog* new_dialog() { return new ScopeDialog(); };

    static inline Singleton<ScopeDialog, ScopeDialog::new_dialog> s_scope_dialog;

    Fl_Slider m_zoom;
    Fl_Button m_reset_zoom;
    Fl_Check_Button m_remove_bias;
    Fl_Check_Button m_normalized;
    Fl_Light_Button m_pause;
    Fl_Button m_capture;
};

}
