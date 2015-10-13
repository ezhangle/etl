//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "test_light.hpp"

TEST_CASE( "optimize/6", "[fast][optimizer]" ) {
    etl::fast_vector<double, 3> a({1.0, -2.0, 3.0});
    etl::fast_vector<double, 3> b;

    b = opt( 0.0 * a + 0.0 * a);

    REQUIRE(b[0] == 0.0);
}

TEST_CASE( "optimize/7", "[fast][optimizer]" ) {
    etl::fast_vector<double, 3> a({1.0, -2.0, 3.0});
    etl::fast_vector<double, 3> b;

    b = opt( 0.0 * a + 0.0 * a + 1.0 * a);

    REQUIRE(b[0] == 1.0);
}

TEST_CASE( "optimize/8", "[fast][optimizer]" ) {
    etl::fast_vector<double, 3> a({1.0, -2.0, 3.0});
    etl::fast_vector<double, 3> b;

    b = opt( 0.0 * a + 1.0 * a + 1.0 * (a - 0));

    REQUIRE(b[0] == 2.0);
}

TEST_CASE( "optimize/10", "[fast][optimizer]" ) {
    etl::fast_vector<double, 3> a({1.0, -2.0, 3.0});
    etl::fast_vector<double, 3> b;

    b = opt( +((-(a * 1.0)) * 1.0));

    REQUIRE(b[0] == -1.0);
}