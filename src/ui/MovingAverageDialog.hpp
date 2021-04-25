#pragma once

#include <Fl/Fl_Spinner.H>
#include <Fl/Fl_Window.H>
#include <pipe/Component.hpp>
#include <ui/component/FrequencyPlot.hpp>
#include <util/Singleton.hpp>

namespace Dsp {
class MovingAverageBase;
}

namespace Ui {

class MovingAverageDialog final : Fl_Window {
public:
    static void show_dialog(ConfigRef<Dsp::MovingAverageBase> filter);
    static void close_dialog();
    static void update_dialog();

private:
    static MovingAverageDialog* new_dialog() { return new MovingAverageDialog(); }
    static inline Singleton<MovingAverageDialog, MovingAverageDialog::new_dialog> s_moving_average_dialog;

    MovingAverageDialog();

    static void update_filter(Fl_Widget*);

    Fl_Group* m_plot_container { nullptr };
    FrequencyPlot* m_plot { nullptr };
    Fl_Group* m_settings_container { nullptr };
    Fl_Spinner* m_taps { nullptr };
};

}
