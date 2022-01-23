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
