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
