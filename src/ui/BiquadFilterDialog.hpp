#pragma once

#include <Fl/Fl_Check_Button.H>
#include <Fl/Fl_Choice.H>
#include <Fl/Fl_Spinner.H>
#include <Fl/Fl_Window.H>
#include <pipe/Component.hpp>
#include <ui/component/FrequencyPlot.hpp>
#include <util/Singleton.hpp>

namespace Dsp::Biquad {
class FilterBase;
}

namespace Ui {

class BiquadFilterDialog final : Fl_Window {
public:
    static void show_dialog(ConfigRef<Dsp::Biquad::FilterBase> filter);
    static void close_dialog();
    static void update_dialog();

private:
    static BiquadFilterDialog* new_dialog() { return new BiquadFilterDialog(); }
    static inline Singleton<BiquadFilterDialog, BiquadFilterDialog::new_dialog> s_biquad_filter_dialog;

    static void update_filter(Fl_Widget*);

    BiquadFilterDialog();

    Fl_Group* m_plot_container { nullptr };
    FrequencyPlot* m_plot { nullptr };
    Fl_Group* m_settings_container { nullptr };
    Fl_Spinner* m_center { nullptr };
    Fl_Spinner* m_parameter { nullptr };
    Fl_Choice* m_type { nullptr };
};

}
