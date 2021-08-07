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

#include "FFMatrix.hpp"
#include "FiniteField.hpp"
#include "Z2Polynomial.hpp"
#include <cmath>
#include <util/Types.hpp>

namespace Bch {

enum class EncodingType : bool {
    Factor,
    Prefix
};

template<EncodingType encoding_type, u8 n, u8 k, u8 t>
class Code final {
    static_assert(n && n >= k);

public:
    using Message = Z2Polynomial::Store;
    using Syndromes = std::array<Z2Polynomial, 2 * t>;

    explicit Code(Z2Polynomial generator)
        : m_generator(std::move(generator))
        , m_field(std::ceil(std::log2(n + 1))) {}

    Message encode(Message message) const {
        Z2Polynomial result(message);
        assert(result.is_zero() || result.degree() < k);

        if constexpr (encoding_type == EncodingType::Factor) {
            return (result * m_generator).coefficients();
        } else if (encoding_type == EncodingType::Prefix) {
            Z2Polynomial code_prefix(message << (n - k));                      // Multiply by x^(n-k)
            return (code_prefix + (code_prefix % m_generator)).coefficients(); // Subtract remainder
        }
    }

    Message decode(Message code_word) const {
        if constexpr (encoding_type == EncodingType::Factor)
            return (Z2Polynomial(code_word) / m_generator).coefficients();
        else if (encoding_type == EncodingType::Prefix)
            return ((code_word >> 1) & ~Z2Polynomial::msb_mask) >> (n - k - 1);
    }

    std::optional<Message> correct(Message code_word) const {
        Z2Polynomial received(code_word);
        if (!received.is_zero() && received.degree() >= n)
            return {};

        bool errors_detected = false;
        Syndromes syndromes;
        for (u8 i = 0; i < syndromes.size(); ++i) {
            const auto syndrome = m_field.syndrome(code_word, i + 1);
            syndromes[i] = syndrome;
            errors_detected |= !syndrome.is_zero();
        }

        if (!errors_detected)
            return code_word;

        const auto coefficients = calculate_error_locator_polynomial(build_syndrome_matrix(syndromes), syndromes);
        if (coefficients.empty())
            return {};

        u8 zeroes_found = 0;
        for (const auto& root : m_field.primitive_roots()) {
            Z2Polynomial sum(1);
            u8 power = 1;

            for (const auto& coefficient : coefficients) {
                sum += m_field.multiply_roots(coefficient, m_field.power_of_root(root, power));
                ++power;
            }

            if (sum.is_zero()) {
                ++zeroes_found;
                u8 location = (m_field.element_count() - m_field.root_exponent(root)) % m_field.element_count();
                code_word ^= 1 << location;
            }
        }

        if (zeroes_found == 0)
            return {};

        return code_word;
    }

private:
    auto build_syndrome_matrix(const Syndromes& syndromes) const {
        FFMatrix<t, t> result(m_field);
        for (u8 r = 0; r < t; ++r) {
            for (u8 c = 0; c < t; ++c)
                result.at(r, c) = syndromes[r + c];
        }

        return result;
    }

    template<u8 rows_left>
    static std::vector<Z2Polynomial> calculate_error_locator_polynomial(FFMatrix<rows_left, rows_left> syndrome_matrix, const Syndromes& syndromes) {
        if constexpr (rows_left == 0) {
            return {};
        } else {
            if (syndrome_matrix.determinant().is_zero())
                return calculate_error_locator_polynomial<rows_left - 1>(syndrome_matrix.shrink(), syndromes);

            FFMatrix<rows_left, 1> syndrome_vector(syndrome_matrix.field());
            for (u8 r = 0; r < rows_left; ++r)
                syndrome_vector.at(r, 0) = syndromes[r + rows_left];

            const auto coefficients = syndrome_matrix.inverse() * syndrome_vector;
            std::vector<Z2Polynomial> result;
            result.resize(rows_left);
            for (u8 i = 0; i < rows_left; ++i)
                result[i] = coefficients.at(rows_left - i - 1, 0);

            return result;
        }
    }

    Z2Polynomial m_generator;
    FiniteField m_field;
};

}
