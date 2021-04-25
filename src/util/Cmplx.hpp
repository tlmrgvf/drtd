#pragma once

#include <cassert>
#include <cmath>
#include <stdint.h>

namespace Util {

class Cmplx {
public:
    constexpr Cmplx() = default;
    constexpr Cmplx(float real, float imag)
        : m_real(real)
        , m_imag(imag) {
    }

    float real() const { return m_real; };
    float imag() const { return m_imag; };
    float angle() const { return std::atan2(m_imag, m_real); }
    float magnitude() const { return std::sqrt(magnitude_squared()); }
    float magnitude_squared() const { return m_real * m_real + m_imag * m_imag; }
    void normalize() {
        float mag = magnitude();
        m_real /= mag;
        m_imag /= mag;
    }

    Cmplx& operator=(float value) {
        m_real = value;
        m_imag = value;
        return *this;
    }

    bool operator==(float value) const { return m_real == value && m_imag == value; }
    Cmplx operator+(const Cmplx& other) const { return Cmplx(m_real + other.m_real, m_imag + other.m_imag); }
    Cmplx& operator+=(const Cmplx& other) {
        m_real += other.m_real;
        m_imag += other.m_imag;
        return *this;
    }
    Cmplx& operator*=(float scale) {
        m_real *= scale;
        m_imag *= scale;
        return *this;
    }

    Cmplx operator-(const Cmplx& other) const { return Cmplx(m_real - other.m_real, m_imag - other.m_imag); }
    Cmplx operator*(const Cmplx& other) const { return Cmplx(m_real * other.m_real - m_imag * other.m_imag, m_real * other.m_imag + m_imag * other.m_real); }
    Cmplx operator*(float scale) const { return Cmplx(m_real * scale, m_imag * scale); }
    Cmplx operator/(float scale) const { return Cmplx(m_real / scale, m_imag / scale); }
    Cmplx operator/(const Cmplx& other) const {
        float os = other.magnitude_squared();
        return Cmplx((m_real * other.m_real + m_imag * other.m_imag) / os, (m_imag * other.m_real - m_real * other.m_imag) / os);
    }

    float operator[](u8 i) const {
        assert(i < 2);
        return i ? m_imag : m_real;
    }

private:
    float m_real { 0 };
    float m_imag { 0 };
};

}

using Util::Cmplx;
