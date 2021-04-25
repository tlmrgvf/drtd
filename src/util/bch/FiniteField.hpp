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
