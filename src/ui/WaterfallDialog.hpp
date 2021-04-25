#pragma once

#include <Fl/Fl_Check_Button.H>
#include <Fl/Fl_Choice.H>
#include <Fl/Fl_Slider.H>
#include <Fl/Fl_Spinner.H>
#include <Fl/Fl_Window.H>
#include <ui/component/Waterfall.hpp>
#include <util/Singleton.hpp>

namespace Ui {

class WaterfallDialog final : Fl_Window {
public:
    static constexpr float s_min_zoom { -10 };
    static constexpr float s_max_zoom { 10 };
    static void show_dialog();
    static void load_from_waterfall(const Waterfall::Settings& settings);

private:
    WaterfallDialog();

    static WaterfallDialog* new_dialog() { return new WaterfallDialog(); };
    static void save_to_waterfall(Fl_Widget*, void*);

    void update_offset_spinner_limits(const Waterfall::Settings& settings);

    static inline Singleton<WaterfallDialog, WaterfallDialog::new_dialog> s_waterfall_dialog;

    Fl_Choice m_window;
    Fl_Slider m_zoom;
    Fl_Button m_reset_zoom;
    Fl_Spinner m_bin_offset;
    Fl_Slider m_speed_multiplier;
    Fl_Choice m_bins;
    Fl_Choice m_palette;
    Fl_Check_Button m_power_spectrum;
};

}
