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

#include <sstream>
#include <stdint.h>

namespace Util {

class Point final {
public:
    constexpr Point() {
    }

    constexpr Point(int x, int y)
        : m_x(x)
        , m_y(y) {
    }

    int x() const { return m_x; }
    int y() const { return m_y; }

    void set_x(int x) { m_x = x; }
    void set_y(int y) { m_y = y; }

    void translate(int dx, int dy) {
        m_x += dx;
        m_y += dy;
    }

    void translate(const Point& point) {
        m_x += point.m_x;
        m_y += point.m_y;
    }

    [[nodiscard]] Point translated(int dx, int dy) const {
        return { m_x + dx, m_y + dy };
    }

private:
    int m_x { 0 };
    int m_y { 0 };
};

[[maybe_unused]] static std::ostream& operator<<(std::ostream& stream, const Util::Point& point) {
    stream << "(" << point.x() << ", " << point.y() << ")";
    return stream;
}

}

using Util::Point;
