#pragma once

#include <Fl/Fl_Check_Button.H>
#include <Fl/Fl_Choice.H>
#include <Fl/Fl_Spinner.H>
#include <Fl/Fl_Window.H>
#include <pipe/Component.hpp>
#include <ui/component/FrequencyPlot.hpp>
#include <util/Singleton.hpp>

namespace Dsp {
class FirFilterBase;
}

namespace Ui {

class FirFilterDialog final : Fl_Window {
public:
    static void show_dialog(ConfigRef<Dsp::FirFilterBase> filter);
    static void close_dialog();
    static void update_dialog();

private:
    static FirFilterDialog* new_dialog() { return new FirFilterDialog(); };
    static inline Singleton<FirFilterDialog, FirFilterDialog::new_dialog> s_fir_filter_dialog;

    FirFilterDialog();

    static void update_filter(Fl_Widget*);

    Fl_Group* m_plot_container { nullptr };
    FrequencyPlot* m_plot { nullptr };
    Fl_Group* m_settings_container { nullptr };
    Fl_Spinner* m_taps { nullptr };
    Fl_Spinner* m_start_frequency { nullptr };
    Fl_Spinner* m_stop_frequency { nullptr };
    Fl_Check_Button* m_invert { nullptr };
    Fl_Choice* m_window { nullptr };
};

}
