#include "TextDisplay.hpp"
#include <Fl/Fl.H>

using namespace Ui;

TextDisplay::TextDisplay(int x, int y, int w, int h)
    : Fl_Text_Display(x, y, w, h) {
    buffer(new Fl_Text_Buffer());
}

void TextDisplay::scroll_to_bottom() {
    scroll(mBuffer->count_lines(0, mBuffer->length()), 0);
}

bool TextDisplay::should_autoscroll() {
    return mVScrollBar->value() == static_cast<int>(mVScrollBar->maximum());
}

void TextDisplay::clear() {
    mBuffer->remove(0, mBuffer->length());
}
