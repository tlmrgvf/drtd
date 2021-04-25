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
