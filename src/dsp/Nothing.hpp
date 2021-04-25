#pragma once

#include <Fl/fl_draw.H>
#include <decoder/Decoder.hpp>
#include <pipe/Component.hpp>
#include <thread>

namespace Dsp {

template<typename T>
class Nothing final : public ComponentBase<T, T> {
public:
    Nothing()
        : ComponentBase<T, T>("Nothing") {
    }

    virtual Size calculate_size() override {
        return size;
    }

protected:
    virtual void draw_at(Point p) override {
        fl_rect(p.x(), p.y(), size.w(), size.h());
        fl_line(p.x(), p.y() + 3, p.x() + 14, p.y() + 3);
        fl_line(p.x(), p.y() + 4, p.x() + 14, p.y() + 4);
    }

    virtual T process(T t) override {
        return t;
    }

private:
    static constexpr Size size { 15, 8 };
};

}
