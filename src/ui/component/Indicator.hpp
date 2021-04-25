#pragma once

#include <Fl/Fl_Box.H>
#include <Fl/Fl_Group.H>

namespace Ui {

class Indicator : public Fl_Group {
public:
    static constexpr Fl_Color yellow_off { 0x50160000 };
    static constexpr Fl_Color yellow_on { 0xDDCC0000 };
    static constexpr Fl_Color green_off { 0x00300000 };
    static constexpr Fl_Color green_on { 0x00CC0000 };
    static constexpr Fl_Color red_off { 0x4C000000 };
    static constexpr Fl_Color red_on { 0xC7000000 };

    Indicator(int x, int y, int w, int h, Fl_Color on_color, Fl_Color off_color, const char* label);

    void set_state(bool state);

private:
    Fl_Box m_indicator;
    Fl_Box m_label;
    bool m_state { false };
    Fl_Color m_on_color;
    Fl_Color m_off_color;
};

}
