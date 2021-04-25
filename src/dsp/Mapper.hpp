/*
BSD 2-Clause License

Copyright (c) 2020, Till Mayer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
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
