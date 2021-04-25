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

#include "Buffer.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>

namespace Util::Config {

void load_file();
void save_all();
void setup(const char* exec_path);

Buffer<char>& buffer(std::string);
bool buffers_contains(std::string);
void make_buffer(std::string k, u16 size);

template<typename T>
void save(std::string key, const T& value) {
    static_assert(std::is_trivially_copyable<T>::value, "Type has to be memcpy-able");
    static_assert(sizeof(T) <= Util::pow2(sizeof(u16) * 8), "Type size too big");

    if (!buffers_contains(key))
        make_buffer(key, sizeof(T));

    auto& existing = buffer(key);
    if (existing.size() != sizeof(T))
        Util::die("Config has duplicate key with different sizes! Outdated save file?");
    std::memcpy(existing.ptr(), &value, sizeof(T));
}

template<typename T>
void load(std::string key, T& value, T default_value) {
    static_assert(std::is_trivially_copyable<T>::value, "Type has to be memcpy-able");
    static_assert(sizeof(T) <= Util::pow2(sizeof(u16) * 8), "Type size too big");

    if (buffers_contains(key)) {
        auto& existing = buffer(key);
        if (existing.size() != sizeof(T))
            Util::die("Config has duplicate key with different sizes! Outdated save file?");

        std::memcpy(&value, existing.ptr(), sizeof(T));
    } else {
        value = default_value;
    }
}

}
