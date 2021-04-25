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
#include "Window.hpp"
#include <array>
#include <cstddef>
#include <util/Util.hpp>

using namespace Dsp;

constexpr Window make_window(WindowType type) {
    switch (type) {
    case WindowType::Rectangular:
        return { "Rectangular",
                 [](Util::Buffer<float>& buffer) {
                     for (size_t i = 0; i < buffer.size(); ++i)
                         buffer[i] = 1;
                 } };
    case WindowType::Blackman:
        return { "Blackman",
                 [](Util::Buffer<float>& buffer) {
                     auto size = buffer.size();
                     for (size_t i = 0; i < size; ++i) {
                         float index = static_cast<float>(i);
                         float a = 0.3635819f - 0.4891775f * cosf((Util::two_pi_f * index) / static_cast<float>(size - 1));
                         float b = 0.1365995f * cosf((2 * Util::two_pi_f * index) / static_cast<float>(size - 1));
                         float c = 0.0106411f * cosf((3 * Util::two_pi_f * index) / static_cast<float>(size - 1));
                         buffer[i] = a + b + c;
                     }
                 } };
    case WindowType::Hamming:
        return { "Hamming",
                 [](Util::Buffer<float>& buffer) {
                     auto size = buffer.size();
                     for (size_t i = 0; i < size; ++i)
                         buffer[i] = 0.53836f - 0.46164f * cosf((Util::two_pi_f * static_cast<float>(i)) / static_cast<float>(size - 1));
                 } };
    case WindowType::Hann:
        return {
            "Hann",
            [](Util::Buffer<float>& buffer) {
                auto size = buffer.size();
                for (size_t i = 0; i < size; ++i) {
                    float c = sinf(Util::pi_f * static_cast<float>(i) / static_cast<float>(size - 1));
                    buffer[i] = c * c;
                }
            }
        };
    default:
        assert(false);
        Util::should_not_be_reached();
    }
}

Window Window::make(WindowType window) {
    return make_window(window);
}

constexpr Window::WindowArray create_all_windows() {
    Window::WindowArray ret;
    for (size_t i = 0; i < static_cast<size_t>(WindowType::__Count); ++i)
        ret[i] = make_window(static_cast<WindowType>(i));

    return ret;
}

const Window::WindowArray Window::s_windows = create_all_windows();
