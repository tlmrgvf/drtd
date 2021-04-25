#pragma once

#include <Fl/Fl.H>
#include <Fl/Fl_Box.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Choice.H>
#include <Fl/Fl_Scroll.H>
#include <Fl/Fl_Window.H>
#include <functional>
#include <sstream>
#include <ui/component/Canvas.hpp>
#include <util/Point.hpp>

namespace Ui {

class ConfigDialog final : Fl_Window {
public:
    static void show_dialog();
    static void refresh();

    ConfigDialog();

private:
    class PipelineCanvas final : public Canvas {
    public:
        PipelineCanvas(u32 x, u32 y, u32 w, u32 h, u8 padding)
            : Canvas(x, y, w, h, padding) {
        }

        virtual int handle(int event) override {
            if (event == FL_PUSH && m_on_click)
                m_on_click({ Fl::event_x() - x() - padding(), Fl::event_y() - y() - padding() }, Fl::event_button(), Fl::event_ctrl());
            return event;
        }

        std::function<void(Util::Point, int, bool)> m_on_click;
    };

    void update(bool reposition);
    void pipeline_click_event(Util::Point, int, bool);

    Fl_Choice m_audio_line;
    Fl_Choice m_monitored_value;
    PipelineCanvas m_pipeline_canvas;
    std::shared_ptr<Layer> m_pipeline_layer;
    Fl_Button m_reset_button;
};

}
