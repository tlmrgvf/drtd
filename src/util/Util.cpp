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
#include "Util.hpp"
#include "Buffer.hpp"
#include "Point.hpp"
#include "Size.hpp"
#include "Types.hpp"
#include <FL/fl_draw.H>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>

static constexpr std::array ascii_escape { "<NUL>", "<SOH>", "<STX>", "<ETX>", "<EOT>", "<ENQ>", "<ACK>", "<BEL>", "<BS>", "<TAB>",
                                           "<LF>", "<VT>", "<FF>", "<CR>", "<SO>", "<SI>", "<DLE>", "<DC1>", "<DC2>", "<DC3>",
                                           "<DC4>", "<NAK>", "<SYN>", "<ETB>", "<CAN>", "<EM>", "<SUB>", "<ESC>", "<FS>", "<GS>",
                                           "<RS>", "<US>" };

std::string Util::to_lower(const std::string& string) {
    std::ostringstream builder;

    for (char c : string) {
        if (c >= 'A' && c <= 'Z')
            builder << static_cast<char>(c + ('a' - 'A'));
        else
            builder << c;
    }

    return builder.str();
}

void Util::die(const char* description) {
    fprintf(stderr, "Fatal error: %s\n", description);
    abort();
}

void Util::should_not_be_reached() {
    die("Reached should_not_be_reached!");
}

std::optional<float> Util::parse_float(const std::string& text) {
    char* last = nullptr;
    long result = std::strtod(text.c_str(), &last);
    if (last == text.c_str())
        return {};

    if (result < std::numeric_limits<float>::min() || result > std::numeric_limits<float>::max())
        return {};

    return static_cast<float>(result);
}

std::optional<int> Util::parse_int(const std::string& text) {
    char* last = nullptr;
    long result = std::strtol(text.c_str(), &last, 10);
    if (last == text.c_str())
        return {};

    if (result < std::numeric_limits<int>::min() || result > std::numeric_limits<int>::max())
        return {};

    return static_cast<int>(result);
}

int Util::center(int total, int size) {
    return (total - size) / 2;
}

float Util::linear_interpolate(float float_index, const Buffer<float>& input_values) {
    assert(float_index >= 0);
    float error = float_index - static_cast<int>(float_index);
    return (error * input_values[std::min(input_values.size() - 1, static_cast<size_t>(float_index + 1))]) + ((1 - error) * input_values[static_cast<size_t>(float_index)]);
}

bool Util::rect_contains(Util::Point check, Util::Point point, Util::Size size) {
    int upper_x = point.x() + size.w();
    int upper_y = point.y() + size.h();
    return check.x() >= point.x() && check.x() <= upper_x && check.y() >= point.y() && check.y() <= upper_y;
}

float Util::db_voltage(float value, float reference) {
    return 20 * log10f(value / reference);
}

float Util::db_power(float value, float reference) {
    return 10 * log10f(value / reference);
}

float Util::scale_log(float value, float source_min, float source_max, bool invert) {
    value = std::clamp(value, source_min, source_max);
    value = (value - source_min) / (source_max - source_min);
    if (invert)
        value = 1 - value;

    float result = (powf(value + 1, 10) - 1) / 1023;
    return invert ? 1 - result : result;
}

std::string Util::escape_ascii(char c) {
    c &= 0x7F;

    if (c > 31 && c < 127)
        return { c };
    else if (c == 127)
        return "<DEL>";

    return ascii_escape[c];
}
