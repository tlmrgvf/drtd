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
