#pragma once

#include <Fl/Fl_Text_Display.H>
#include <memory>

namespace Ui {

class TextDisplay final : public Fl_Text_Display {
public:
    TextDisplay(int x, int y, int w, int h);

    bool should_autoscroll();
    void scroll_to_bottom();
    void clear();
};
}
