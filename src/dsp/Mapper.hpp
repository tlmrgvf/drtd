#pragma once

#include <FL/fl_draw.H>
#include <functional>
#include <pipe/Component.hpp>

namespace Dsp {

template<typename In, typename Out>
class Mapper final : public ComponentBase<In, Out> {
public:
    Mapper(std::function<Out(In)> map_function)
        : ComponentBase<In, Out>("Mapper")
        , m_map_function(map_function) {
    }

    virtual Size calculate_size() override {
        return size;
    }

protected:
    virtual void draw_at(Point p) override {
        fl_rect(p.x(), p.y(), size.w(), size.h());
        p.translate(3, 3);
        auto resized = size;
        resized.resize(-7, -7);

        const unsigned position = static_cast<unsigned>(resized.w() * .7);

        for (int i = 0; i < 3; ++i) {
            fl_line(position - i + p.x(), p.y(), resized.w() - i + p.x(), resized.h() / 2 + p.y());
            fl_line(position - i + p.x(), resized.h() + p.y(), resized.w() - i + p.x(), resized.h() / 2 + p.y() + 1);
        }

        fl_line(p.x(), resized.h() / 2 + p.y(), resized.w() + p.x(), resized.h() / 2 + p.y());
        fl_line(p.x(), resized.h() / 2 + p.y() + 1, resized.w() + p.x(), resized.h() / 2 + 1 + p.y());
        fl_line(p.x(), p.y(), p.x(), resized.h() + p.y());
        fl_line(p.x() + 1, p.y(), p.x() + 1, resized.h() + p.y());
    }

    virtual Out process(In in) override {
        return m_map_function(in);
    }

private:
    static constexpr Size size { 20, 16 };

    std::function<Out(In)> m_map_function;
};

}
