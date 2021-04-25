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

#include <array>
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <util/Buffer.hpp>

namespace Dsp {

enum class WindowType {
    Rectangular,
    Blackman,
    Hamming,
    Hann,
    __Count
};

class Window final {
public:
    using WindowArray = std::array<Window, static_cast<size_t>(WindowType::__Count)>;

    Window() = default;
    constexpr Window(const char* name, void (*calculator)(Util::Buffer<float>&))
        : m_name(name)
        , m_calculator(calculator) {
    }

    static Window make(WindowType);

    void calculate_coefficients(Util::Buffer<float>& buffer) const {
        if (m_calculator)
            m_calculator(buffer);
    };

    std::string name() const { return m_name; }

    static const WindowArray s_windows;

private:
    const char* m_name { "" };
    void (*m_calculator)(Util::Buffer<float>&) { nullptr };
};

}
