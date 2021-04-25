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

#include "Types.hpp"
#include <Fl/Fl.H>
#include <cmath>
#include <memory>
#include <optional>
#include <stdint.h>

namespace Util {

class Point;
class Size;
template<typename T>
class Buffer;

static const Fl_Color s_amber_color { fl_rgb_color(250, 213, 14) };

enum class IterationDecision {
    Continue,
    Break
};

template<typename T>
struct TypeIdentity {
    using Type = T;
};

[[noreturn]] void die(const char* description);
[[noreturn]] void should_not_be_reached();
int center(int total, int size);
float linear_interpolate(float floatIndex, const Buffer<float>& inputValues);
bool rect_contains(Point check, Point point, Size size);
float db_voltage(float value, float reference);
float scale_log(float value, float source_min, float source_max, bool invert = false);
std::optional<int> parse_int(const std::string&);
std::optional<float> parse_float(const std::string&);
std::string escape_ascii(char);
std::string to_lower(const std::string&);

constexpr u64 pow2(u8 exponent) {
    return 1 << exponent;
}

static constexpr float pi_f { static_cast<float>(M_PI) };
static constexpr float two_pi_f { static_cast<float>(2 * M_PI) };

}
