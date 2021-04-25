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
