#include "Indicator.hpp"
#include <Fl/fl_draw.H>
#include <util/Size.hpp>
#include <util/Util.hpp>

using namespace Ui;

static constexpr Size indicator_size(25, 15);

Indicator::Indicator(int x, int y, int w, int h, Fl_Color on_color, Fl_Color off_color, const char* label)
    : Fl_Group(x, y, w, h)
    , m_indicator(x + Util::center(w, indicator_size.w()), y + 4, indicator_size.w(), indicator_size.h())
    , m_label(x, m_indicator.y() + m_indicator.h() + 2, w, 13, label)
    , m_on_color(on_color)
    , m_off_color(off_color) {
    m_indicator.box(FL_DOWN_BOX);
    m_label.labelsize(13);
    m_label.labelfont(FL_BOLD);
    resizable(nullptr);
    m_indicator.color(m_off_color);
    end();
}

void Indicator::set_state(bool state) {
    if (state == m_state)
        return;

    m_state = state;
    m_indicator.color(m_state ? m_on_color : m_off_color);
    m_indicator.redraw();
    m_indicator.damage(FL_DAMAGE_ALL);
}
