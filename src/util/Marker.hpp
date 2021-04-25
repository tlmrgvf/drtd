#pragma once

#include "Types.hpp"
#include <stdint.h>
#include <vector>

namespace Util {

struct Marker {
    i32 offset { 0 };
    Hertz bandwidth { 0 };
};

struct MarkerGroup {
    std::vector<Marker> markers {};
    bool moveable { false };
};

}
