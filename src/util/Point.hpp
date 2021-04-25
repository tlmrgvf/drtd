#pragma once

#include <sstream>
#include <stdint.h>

namespace Util {

class Point final {
public:
    constexpr Point() {
    }

    constexpr Point(int x, int y)
        : m_x(x)
        , m_y(y) {
    }

    int x() const { return m_x; }
    int y() const { return m_y; }

    void set_x(int x) { m_x = x; }
    void set_y(int y) { m_y = y; }

    void translate(int dx, int dy) {
        m_x += dx;
        m_y += dy;
    }

    void translate(const Point& point) {
        m_x += point.m_x;
        m_y += point.m_y;
    }

    [[nodiscard]] Point translated(int dx, int dy) const {
        return { m_x + dx, m_y + dy };
    }

private:
    int m_x { 0 };
    int m_y { 0 };
};

[[maybe_unused]] static std::ostream& operator<<(std::ostream& stream, const Util::Point& point) {
    stream << "(" << point.x() << ", " << point.y() << ")";
    return stream;
}

}

using Util::Point;
