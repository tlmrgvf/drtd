#include "FiniteField.hpp"

using namespace Bch;

FiniteField::FiniteField(u8 exponent)
    : m_exponent(exponent) {
    assert(exponent < Z2Polynomial::coefficient_count);
    s_log.info() << "Starting search for an irreducible polynomial of degree " << static_cast<int>(exponent) << "...";
    Z2Polynomial::Store search = 1 << exponent;
    std::optional<Z2Polynomial> irreducible;

    /*
     * REALLY primitive way of searching for an irreducible polynomial, basically just like the primitive prime
     * number check, but the exponent should not get too big with BCH codes and this is only calculated once
     */
    for (Z2Polynomial::Store i = 0; i < search; ++i) {
        const Z2Polynomial candidate(search ^ i);
        bool is_irreducible = true;

        /* Start at x^1 as every polynomial is divisible by 1 */
        for (Z2Polynomial::Store test = 2; test < (search ^ (search - 1)); ++test) {
            const auto remainder = candidate % test;
            if ((!remainder.is_zero() && test == candidate.coefficients()) || (remainder.is_zero() && test != candidate.coefficients())) {
                is_irreducible = false;
                break;
            }
        }

        if (is_irreducible) {
            s_log.info() << "Found irreducible polynomial 0x" << std::hex << candidate.coefficients();
            irreducible = candidate;
            break;
        }
    }

    /* This should never fail, I think? */
    assert(irreducible.has_value());

    m_element_count = Util::pow2(exponent) - 1;
    m_primitive_roots = Buffer<Z2Polynomial>(m_element_count);
    m_primitive_roots_log = Buffer<size_t>(m_element_count + 1);

    Z2Polynomial::Store alpha = 1; //Use x as the primitive root of unity
    m_primitive_roots[0] = alpha;  //a^0
    m_primitive_roots_log[0] = -1; //There is no exponent to make a^n == 0!

    for (size_t i = 1; i < m_element_count; ++i) {
        alpha <<= 1; //times x
        m_primitive_roots[i] = (Z2Polynomial(alpha) % irreducible.value()).coefficients();
        m_primitive_roots_log[m_primitive_roots[i].coefficients()] = i;
    }
}

Z2Polynomial FiniteField::multiply_roots(const Z2Polynomial& multiplier, const Z2Polynomial& multiplicand) const {
    if (multiplier.is_zero() || multiplicand.is_zero())
        return {};
    assert(multiplier.degree() < m_exponent && multiplicand.degree() < m_exponent);

    const auto alpha_exponent = m_primitive_roots_log[multiplier.coefficients()] + m_primitive_roots_log[multiplicand.coefficients()];
    return m_primitive_roots[alpha_exponent % m_element_count];
}

Z2Polynomial FiniteField::power_of_root(const Z2Polynomial& root, u8 exponent) const {
    assert(root.degree() < m_exponent);
    const auto alpha_exponent = m_primitive_roots_log[root.coefficients()];
    return m_primitive_roots[(alpha_exponent * exponent) % m_element_count];
}

size_t FiniteField::root_exponent(const Z2Polynomial& root) const {
    assert(root.degree() < m_exponent);
    return m_primitive_roots_log[root.coefficients()];
}

Z2Polynomial FiniteField::power_of_x(u8 exponent) const {
    return m_primitive_roots[exponent % m_element_count];
}

Z2Polynomial FiniteField::root_inverse(const Z2Polynomial& root) const {
    assert(!root.is_zero()); //There is no multiplicative inverse to 0

    const auto alpha_exponent = m_primitive_roots_log[root.coefficients()];
    return m_primitive_roots[(m_element_count - alpha_exponent) % m_element_count];
}

Z2Polynomial FiniteField::syndrome(const Z2Polynomial& polynomial, u8 n) const {
    Z2Polynomial result;
    for (auto exponent : polynomial.exponent_values())
        result += power_of_x(exponent * n);

    return result;
}
