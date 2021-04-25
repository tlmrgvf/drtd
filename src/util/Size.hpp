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

#include <cassert>
#include <sstream>
#include <stdint.h>

namespace Util {

class Size final {
public:
    constexpr Size() {
    }

    constexpr Size(unsigned width, unsigned height)
        : m_width(width)
        , m_height(height) {
    }

    constexpr unsigned w() const { return m_width; }
    constexpr unsigned h() const { return m_height; }

    void set_width(unsigned width) { m_width = width; }
    void set_height(unsigned height) { m_height = height; }

    void resize(int dw, int dh) {
        m_width += dw;
        m_height += dh;
    }

    [[nodiscard]] Size resized(int dw, int dh) const {
        return Size(m_width + dw, m_height + dh);
    }

private:
    unsigned m_width { 0 };
    unsigned m_height { 0 };
};

[[maybe_unused]] static std::ostream& operator<<(std::ostream& stream, const Util::Size& size) {
    stream << "(" << size.w() << " x " << size.h() << ")";
    return stream;
}

}

using Util::Size;
