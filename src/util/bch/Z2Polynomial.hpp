#pragma once

#include <cassert>
#include <util/Types.hpp>
#include <util/Util.hpp>
#include <vector>

namespace Bch {

class Z2Polynomial final {
public:
    using Store = u64;
    static constexpr unsigned coefficient_count = sizeof(Store) * 8;
    static constexpr Store msb_mask = static_cast<Store>(1) << (coefficient_count - 1);

    Z2Polynomial() = default;
    Z2Polynomial(Store coefficients);

    enum class DivideResult : bool {
        Quotient,
        Remainder
    };

    static Z2Polynomial poly_divide(const Z2Polynomial& dividend, const Z2Polynomial& divisor, DivideResult);

    const std::vector<u8>& exponent_values() const;
    bool is_zero() const { return m_coefficients == 0; };
    Store coefficients() const { return m_coefficients; };
    u8 degree() const {
        assert(!is_zero());
        return m_degree;
    };

    Z2Polynomial operator+(const Z2Polynomial& right) const { return { right.m_coefficients ^ m_coefficients }; }
    Z2Polynomial& operator+=(const Z2Polynomial& right);
    Z2Polynomial operator*(const Z2Polynomial&) const;
    Z2Polynomial operator%(const Z2Polynomial&) const;
    Z2Polynomial operator/(const Z2Polynomial&) const;

    void calculate_degree();

private:
    Store m_coefficients { 0 };
    u8 m_degree { 0 };
    mutable std::vector<u8> m_exponents;
};

}
