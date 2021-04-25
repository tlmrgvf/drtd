#pragma once

#include <chrono>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

namespace Util {

class Limiter final {
public:
    Limiter(u8 max_updates_per_second)
        : m_min_time_microseconds(1000000 / max_updates_per_second)
        , m_last_time(std::chrono::steady_clock::now()) {
    }

    bool limit() {
        using namespace std::chrono;

        auto now = steady_clock::now();
        if (duration_cast<microseconds>(now - m_last_time).count() < m_min_time_microseconds)
            return false;

        m_last_time = now;
        return true;
    }

private:
    u32 m_min_time_microseconds;
    std::chrono::time_point<std::chrono::steady_clock> m_last_time;
};

}

using Util::Limiter;
