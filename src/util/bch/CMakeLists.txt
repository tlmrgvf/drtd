set(SOURCES
    Z2Polynomial.hpp
    Z2Polynomial.cpp
    FiniteField.hpp
    FiniteField.cpp
    FFMatrix.hpp
    BchCode.hpp)
add_library(bch ${SOURCES})
