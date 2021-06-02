#pragma once

#include <pipe/Component.hpp>
#include <functional>
#include <Fl/fl_draw.H>

namespace Dsp {

template<typename T>
class Tap final : public ComponentBase<T, T> {
public:
    Tap(std::function<void(T)> tap_function)
        : ComponentBase<T, T>("Tap"),
        m_tap_function(tap_function){
    }

    virtual Size calculate_size() override {
        return size;
    }

protected:
    virtual void draw_at(Point p) override {
        fl_line_style(FL_CAP_SQUARE, 2);
        fl_line(p.x(), p.y() + 5, p.x() + 9, p.y() + 5);
        fl_line(p.x() + 5, p.y() + 5, p.x() + 5, p.y() + 0);
        fl_line_style(0);
    }

    virtual T process(T t) override {
        m_tap_function(t);
        return t;
    }

private:
    static constexpr Size size { 10, 10 };

    std::function<void(T)> m_tap_function;
};

};
