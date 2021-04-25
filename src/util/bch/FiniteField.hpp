#pragma once

#include "Z2Polynomial.hpp"
#include <cassert>
#include <util/Buffer.hpp>
#include <util/Logger.hpp>
#include <util/Types.hpp>

namespace Bch {

class FiniteField final {
public:
    FiniteField(u8 exponent);

    const Buffer<Z2Polynomial> primitive_roots() const { return m_primitive_roots; }
    size_t element_count() const { return m_element_count; }
    size_t root_exponent(const Z2Polynomial& root) const;
    Z2Polynomial multiply_roots(const Z2Polynomial& multiplier, const Z2Polynomial& multiplicand) const;
    Z2Polynomial power_of_root(const Z2Polynomial& root, u8 exponent) const;
    Z2Polynomial power_of_x(u8 exponent) const;
    Z2Polynomial root_inverse(const Z2Polynomial& root) const;
    Z2Polynomial syndrome(const Z2Polynomial& polynomial, u8 n) const;

private:
    static inline Logger s_log { "Finite field" };

    u8 m_exponent;
    Buffer<Z2Polynomial> m_primitive_roots;
    Buffer<size_t> m_primitive_roots_log;
    size_t m_element_count;
};

}
