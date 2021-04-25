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
