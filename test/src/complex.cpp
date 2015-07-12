//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "test_light.hpp"

TEMPLATE_TEST_CASE_2( "complex/1", "[complex]", Z, float, double ) {
    etl::fast_vector<std::complex<Z>, 3> a = {-1.0, 2.0, 5.0};

    REQUIRE(a[0].real() == Approx(Z(-1.0)));
    REQUIRE(a[0].imag() == Approx(Z(0.0)));
    REQUIRE(a[1].real() == Approx(Z(2.0)));
    REQUIRE(a[2].imag() == Approx(Z(0.0)));
    REQUIRE(a[2].real() == Approx(Z(5.0)));
    REQUIRE(a[2].imag() == Approx(Z(0.0)));

    a[0] = 33.0;

    REQUIRE(a[0].real() == Approx(Z(33.0)));
    REQUIRE(a[0].imag() == Approx(Z(0.0)));

    a[0].imag(12.0);

    REQUIRE(a[0].real() == Approx(Z(33.0)));
    REQUIRE(a[0].imag() == Approx(Z(12.0)));

    a[0] = std::complex<Z>(1.0, 2.0);

    REQUIRE(a[0].real() == Approx(Z(1.0)));
    REQUIRE(a[0].imag() == Approx(Z(2.0)));

    a = std::complex<Z>(3.0, -2.0);

    REQUIRE(a[0].real() == Approx(Z(3.0)));
    REQUIRE(a[0].imag() == Approx(Z(-2.0)));
    REQUIRE(a[1].real() == Approx(Z(3.0)));
    REQUIRE(a[1].imag() == Approx(Z(-2.0)));
    REQUIRE(a[2].real() == Approx(Z(3.0)));
    REQUIRE(a[2].imag() == Approx(Z(-2.0)));
}

TEMPLATE_TEST_CASE_2( "complex/2", "[complex]", Z, float, double ) {
    etl::fast_vector<std::complex<Z>, 3> a = {std::complex<Z>(1.0, 2.0), std::complex<Z>(-1.0, -2.0), std::complex<Z>(0.0, 0.5)};
    etl::fast_vector<std::complex<Z>, 3> b = {std::complex<Z>(0.33, 0.66), std::complex<Z>(-1.5, 0.0), std::complex<Z>(0.5, 0.75)};
    etl::fast_vector<std::complex<Z>, 3> c;

    c = a >> b;

    REQUIRE(c[0] == a[0] * b[0]);
    REQUIRE(c[1] == a[1] * b[1]);
    REQUIRE(c[2] == a[2] * b[2]);
}

TEMPLATE_TEST_CASE_2( "complex/3", "[complex]", Z, float, double ) {
    etl::fast_vector<std::complex<Z>, 3> a = {std::complex<Z>(1.0, 2.0), std::complex<Z>(-1.0, -2.0), std::complex<Z>(0.0, 0.5)};
    etl::fast_vector<std::complex<Z>, 3> b = {std::complex<Z>(0.33, 0.66), std::complex<Z>(-1.5, 0.0), std::complex<Z>(0.5, 0.75)};
    etl::fast_vector<std::complex<Z>, 3> c;

    c = a + b;

    REQUIRE(c[0].real() == Approx(Z(1.33)));
    REQUIRE(c[0].imag() == Approx(Z(2.66)));
    REQUIRE(c[1].real() == Approx(Z(-2.5)));
    REQUIRE(c[1].imag() == Approx(Z(-2.0)));
    REQUIRE(c[2].real() == Approx(Z(0.5)));
    REQUIRE(c[2].imag() == Approx(Z(1.25)));
}

TEMPLATE_TEST_CASE_2( "complex/4", "[complex]", Z, float, double ) {
    etl::fast_vector<std::complex<Z>, 3> a = {std::complex<Z>(1.0, 2.0), std::complex<Z>(-1.0, -2.0), std::complex<Z>(0.0, 0.5)};
    etl::fast_vector<std::complex<Z>, 3> b = {std::complex<Z>(0.33, 0.66), std::complex<Z>(-1.5, 0.0), std::complex<Z>(0.5, 0.75)};
    etl::fast_vector<std::complex<Z>, 3> c;

    c = a - b;

    REQUIRE(c[0].real() == Approx(Z(0.67)));
    REQUIRE(c[0].imag() == Approx(Z(1.34)));
    REQUIRE(c[1].real() == Approx(Z(0.5)));
    REQUIRE(c[1].imag() == Approx(Z(-2.0)));
    REQUIRE(c[2].real() == Approx(Z(-0.5)));
    REQUIRE(c[2].imag() == Approx(Z(-0.25)));
}

TEMPLATE_TEST_CASE_2( "complex/5", "[complex]", Z, float, double ) {
    etl::fast_vector<std::complex<Z>, 3> a = {std::complex<Z>(1.0, 2.0), std::complex<Z>(-1.0, -2.0), std::complex<Z>(0.0, 0.5)};
    etl::fast_vector<std::complex<Z>, 3> b = {std::complex<Z>(0.33, 0.66), std::complex<Z>(-1.5, 0.0), std::complex<Z>(0.5, 0.75)};
    etl::fast_vector<std::complex<Z>, 3> c;

    c = a >> b;

    REQUIRE(c[0].real() == Approx(Z(-0.99)));
    REQUIRE(c[0].imag() == Approx(Z(1.32)));
    REQUIRE(c[1].real() == Approx(Z(1.5)));
    REQUIRE(c[1].imag() == Approx(Z(3.0)));
    REQUIRE(c[2].real() == Approx(Z(-0.375)));
    REQUIRE(c[2].imag() == Approx(Z(0.25)));
}

TEMPLATE_TEST_CASE_2( "complex/6", "[complex]", Z, float, double ) {
    etl::fast_vector<std::complex<Z>, 3> a = {std::complex<Z>(1.0, 2.0), std::complex<Z>(-1.0, -2.0), std::complex<Z>(0.0, 0.5)};
    etl::fast_vector<std::complex<Z>, 3> b = {std::complex<Z>(0.33, 0.66), std::complex<Z>(-1.5, 0.0), std::complex<Z>(0.5, 0.75)};
    etl::fast_vector<std::complex<Z>, 3> c;

    c = a / b;

    REQUIRE(c[0].real() == Approx(Z(3.0303030)));
    REQUIRE(c[0].imag() == Approx(Z(0.0)));
    REQUIRE(c[1].real() == Approx(Z(0.6666666)));
    REQUIRE(c[1].imag() == Approx(Z(1.3333333)));
    REQUIRE(c[2].real() == Approx(Z(0.461538)));
    REQUIRE(c[2].imag() == Approx(Z(0.3076923)));
}