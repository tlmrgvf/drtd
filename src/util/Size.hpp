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
