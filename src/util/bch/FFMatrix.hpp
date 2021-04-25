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

#include "FiniteField.hpp"
#include "Z2Polynomial.hpp"
#include <array>
#include <cassert>
#include <util/Types.hpp>

namespace Bch {

namespace Permutation {

constexpr u64 faculty(u8 n) {
    u64 faculty = 1;
    for (u8 i = 1; i <= n; ++i)
        faculty *= i;

    return faculty;
}

template<u8 n>
constexpr std::array<u8, n * faculty(n)> calculate_permutations() {
    const auto permutation_count = faculty(n);
    std::array<u8, n * permutation_count> result {};
    const auto perm_count_minus_one = faculty(n - 1);
    const auto perms_minus_one = calculate_permutations<n - 1>();

    for (size_t perm = 0; perm < permutation_count; ++perm) {
        const auto flip = perm / perm_count_minus_one;
        const auto previous_offset = (perm % perm_count_minus_one) * (n - 1);
        const auto result_offset = perm * n;

        for (size_t i = 0; i < n - 1; ++i)
            result[i + result_offset] = perms_minus_one[i + previous_offset];

        if (flip) {
            result[result_offset + n - 1] = result[result_offset + flip - 1];
            result[result_offset + flip - 1] = n;
        } else {
            result[result_offset + n - 1] = n;
        }
    }

    return result;
}

template<>
constexpr std::array<u8, 1> calculate_permutations<1>() {
    return { 1 };
}

template<>
constexpr std::array<u8, 0> calculate_permutations<0>() {
    return {};
}

}

template<u8 rows, u8 columns>
class FFMatrix final {
public:
    static constexpr u16 element_count = rows * columns;
    using Store = std::array<Z2Polynomial, element_count>;

    FFMatrix(FiniteField field, Store elements)
        : m_elements(std::move(elements))
        , m_field(std::move(field)) {}

    explicit FFMatrix(FiniteField field)
        : m_elements({})
        , m_field(std::move(field)) {}

    constexpr u8 row_count() const { return rows; };
    constexpr u8 column_count() const { return columns; };

    const Z2Polynomial& at(u8 row, u8 column) const {
        assert(row < rows && column < columns);
        return m_elements[column + row * columns];
    }

    Z2Polynomial& at(u8 row, u8 column) {
        assert(row < rows && column < columns);
        return m_elements[column + row * columns];
    }

    const FiniteField& field() const { return m_field; }

    const Store& elements() const { return m_elements; }

    FFMatrix inverse() const {
        static_assert(rows == columns);
        return adjugate() * m_field.root_inverse(determinant());
    }

    FFMatrix<std::max(0, rows - 1), std::max(0, columns - 1)> shrink() const {
        static_assert(rows && columns && rows == columns);
        if constexpr (rows <= 1)
            return FFMatrix<0, 0>(m_field, {});

        FFMatrix<rows - 1, columns - 1> result(m_field);
        for (u8 r = 0; r < rows - 1; ++r) {
            for (u8 c = 0; c < columns - 1; ++c)
                result.at(r, c) = at(r, c);
        }

        return result;
    }

    FFMatrix adjugate() const {
        static_assert(rows == columns);
        if constexpr (rows == 0) {
            return FFMatrix<0, 0>(m_field);
        } else if constexpr (rows == 1) {
            return FFMatrix<1, 1>(m_field, { { at(0, 0).is_zero() ? 0 : 1 } });
        } else if constexpr (rows == 2) {
            return FFMatrix<2, 2>(m_field, { at(1, 1), at(0, 1), at(1, 0), at(0, 0) });
        } else {
            FFMatrix result(m_field);
            for (u8 r = 0; r < rows; ++r) {
                for (u8 c = 0; c < columns; ++c)
                    result.at(r, c) = minor(c, r).determinant(); //Flip r and c to transpose
            }

            return result;
        }
    }

    FFMatrix<std::max(0, rows - 1), std::max(0, columns - 1)> minor(u8 rr, u8 cr) const {
        static_assert(rows && columns);
        FFMatrix<rows - 1, columns - 1> result(m_field);
        u8 new_row = 0;
        u8 new_column = 0;

        for (u8 r = 0; r < rows; ++r) {
            if (rr == r)
                continue;

            for (u8 c = 0; c < columns; ++c) {
                if (cr != c)
                    result.at(new_row, new_column++) = at(r, c);
            }

            new_column = 0;
            ++new_row;
        }

        return result;
    }

    Z2Polynomial determinant() const {
        static_assert(rows && columns && rows == columns);

        if (rows == 1)
            return at(0, 0);

        Z2Polynomial result;
        for (size_t permutation = 0; permutation < Permutation::faculty(rows); ++permutation) {
            Z2Polynomial product(1);
            for (u8 i = 0; i < rows; ++i)
                product = m_field.multiply_roots(at(i, permutations[permutation * rows + i] - 1), product);

            result += product;
        }

        return result;
    }

    FFMatrix operator+(const FFMatrix& right) const {
        FFMatrix result(m_field);
        for (u8 r = 0; r < rows; ++r) {
            for (u8 c = 0; c < columns; ++c)
                result.at(r, c) = at(r, c) + right.at(r, c);
        }

        return result;
    }

    FFMatrix operator*(const Z2Polynomial& right) const {
        FFMatrix result(m_field);
        for (u8 r = 0; r < rows; ++r) {
            for (u8 c = 0; c < columns; ++c)
                result.at(r, c) = m_field.multiply_roots(at(r, c), right);
        }

        return result;
    }

    template<u8 right_column_count>
    auto operator*(const FFMatrix<columns, right_column_count>& right) const {
        FFMatrix<rows, right_column_count> result(m_field);
        for (u8 r = 0; r < rows; ++r) {
            for (u8 c = 0; c < right_column_count; ++c) {
                Z2Polynomial sum;
                for (u8 sc = 0; sc < columns; ++sc)
                    sum += m_field.multiply_roots(at(r, sc), right.at(sc, c));

                result.at(r, c) = sum;
            }
        }

        return result;
    }

private:
    static constexpr auto permutations = Permutation::calculate_permutations<rows>();

    Store m_elements; //row major ordering
    FiniteField m_field;
};

}
