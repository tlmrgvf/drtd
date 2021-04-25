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
#include "Z2Polynomial.hpp"
#include <cassert>
#include <cmath>

using namespace Bch;

Z2Polynomial::Z2Polynomial(Store coefficients)
    : m_coefficients(coefficients) {
    calculate_degree();
}

Z2Polynomial Z2Polynomial::poly_divide(const Z2Polynomial& dividend, const Z2Polynomial& divisor, Z2Polynomial::DivideResult result) {
    if (dividend.is_zero())
        return {};
    assert(!divisor.is_zero());

    Store quotient_bit = dividend.degree() - divisor.degree();
    Store divisor_shift = divisor.m_coefficients << quotient_bit;
    Store check_bit = 1 << dividend.degree();
    Store dividing = dividend.m_coefficients;
    Store quotient = 0;
    const auto steps = quotient_bit;

    for (Store i = 0; i <= steps; ++i) {
        if (dividing & check_bit) {
            quotient |= 1 << quotient_bit;
            dividing ^= divisor_shift;
        }

        divisor_shift >>= 1;
        check_bit >>= 1;
        --quotient_bit;
    }

    return { result == DivideResult::Remainder ? dividing : quotient };
}

void Z2Polynomial::calculate_degree() {
    if (is_zero())
        m_degree = 0;
    else
        m_degree = static_cast<u8>(std::log2(m_coefficients));
}

Z2Polynomial& Z2Polynomial::operator+=(const Z2Polynomial& right) {
    m_coefficients ^= right.m_coefficients;
    calculate_degree();
    m_exponents.clear();
    return *this;
}

const std::vector<u8>& Z2Polynomial::exponent_values() const {
    if (is_zero())
        return m_exponents;

    if (m_exponents.size())
        return m_exponents;

    Store shift = m_coefficients;
    m_exponents.resize(coefficient_count);

    for (int i = 0; i <= m_degree; ++i) {
        if (shift & 1)
            m_exponents.push_back(i);
        shift >>= 1;
    }

    return m_exponents;
}

Z2Polynomial Z2Polynomial::operator*(const Z2Polynomial& right) const {
    if (is_zero() || right.is_zero())
        return {};

    if (m_degree == 0 || right.m_degree == 0)
        return m_degree == 0 ? right : *this;

    if (m_degree == 1 || right.m_degree == 1)
        return { m_degree == 1 ? Z2Polynomial(right.m_coefficients << 1) : Z2Polynomial(m_coefficients << 1) };

    assert(m_degree + right.m_degree < coefficient_count);

    Store result = 0;
    for (auto exponent : exponent_values()) {
        for (auto other_exponent : right.exponent_values())
            result ^= 1 << (exponent + other_exponent);
    }

    return { result };
}

Z2Polynomial Z2Polynomial::operator%(const Z2Polynomial& divisor) const {
    if (divisor.m_degree > m_degree)
        return *this;

    return poly_divide(*this, divisor, DivideResult::Remainder);
}

Z2Polynomial Z2Polynomial::operator/(const Z2Polynomial& divisor) const {
    if (divisor.m_degree > m_degree)
        return {};

    return poly_divide(*this, divisor, DivideResult::Quotient);
}
