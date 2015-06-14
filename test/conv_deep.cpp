//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "catch.hpp"
#include "template_test.hpp"

#include "etl/etl.hpp"

#include "conv_test.hpp"

//Note: The results of the tests have been validated with one of (octave/matlab/matlab)

//{{{ convolution_deep_full

TEMPLATE_TEST_CASE_2( "convolution_3d/full_1", "convolution_deep_full", Z, float, double ) {
    etl::fast_matrix<Z, 2, 2, 2> a = {1.0, 2.0, 3.0, 2.0, 5.0, 6.0, 7.0, 8.0};
    etl::fast_matrix<Z, 2, 2, 2> b = {2.0, 1.0, 0.5, 0.5, 1.0, 2.0, 1.0, 2.0};
    etl::fast_matrix<Z, 2, 3, 3> c;

    *etl::conv_deep_full(a, b, c);

    REQUIRE(c(0,0,0) == 2.0);
    REQUIRE(c(0,0,1) == 5.0);
    REQUIRE(c(0,0,2) == 2.0);

    REQUIRE(c(0,1,0) == 6.5);
    REQUIRE(c(0,1,1) == 8.5);
    REQUIRE(c(0,1,2) == 3.0);

    REQUIRE(c(0,2,0) == 1.5);
    REQUIRE(c(0,2,1) == 2.5);
    REQUIRE(c(0,2,2) == 1.0);

    REQUIRE(c(1,0,0) == 5.0);
    REQUIRE(c(1,0,1) == 16.0);
    REQUIRE(c(1,0,2) == 12.0);

    REQUIRE(c(1,1,0) == 12.0);
    REQUIRE(c(1,1,1) == 38.0);
    REQUIRE(c(1,1,2) == 28.0);

    REQUIRE(c(1,2,0) == 7.0);
    REQUIRE(c(1,2,1) == 22.0);
    REQUIRE(c(1,2,2) == 16.0);
}

TEMPLATE_TEST_CASE_2( "convolution_4d/full_1", "convolution_deep_full", Z, float, double ) {
    etl::fast_matrix<Z, 1, 2, 2, 2> a = {1.0, 2.0, 3.0, 2.0, 5.0, 6.0, 7.0, 8.0};
    etl::fast_matrix<Z, 1, 2, 2, 2> b = {2.0, 1.0, 0.5, 0.5, 1.0, 2.0, 1.0, 2.0};
    etl::fast_matrix<Z, 1, 2, 3, 3> c;

    c = etl::conv_deep_full(a, b);

    REQUIRE(c(0,0,0,0) == 2.0);
    REQUIRE(c(0,0,0,1) == 5.0);
    REQUIRE(c(0,0,0,2) == 2.0);

    REQUIRE(c(0,0,1,0) == 6.5);
    REQUIRE(c(0,0,1,1) == 8.5);
    REQUIRE(c(0,0,1,2) == 3.0);

    REQUIRE(c(0,0,2,0) == 1.5);
    REQUIRE(c(0,0,2,1) == 2.5);
    REQUIRE(c(0,0,2,2) == 1.0);

    REQUIRE(c(0,1,0,0) == 5.0);
    REQUIRE(c(0,1,0,1) == 16.0);
    REQUIRE(c(0,1,0,2) == 12.0);

    REQUIRE(c(0,1,1,0) == 12.0);
    REQUIRE(c(0,1,1,1) == 38.0);
    REQUIRE(c(0,1,1,2) == 28.0);

    REQUIRE(c(0,1,2,0) == 7.0);
    REQUIRE(c(0,1,2,1) == 22.0);
    REQUIRE(c(0,1,2,2) == 16.0);
}

//}}}