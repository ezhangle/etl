//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_VIRTUAL_VIEWS_HPP
#define ETL_VIRTUAL_VIEWS_HPP

#include <iostream>     //For stream support

#include "cpp_utils/tmp.hpp"
#include "tmp.hpp"

namespace etl {

namespace detail {

template<typename V>
constexpr V compute(std::size_t n, std::size_t i, std::size_t j){
    if(n == 1){
        return 1;
    } else if(n == 2){
        return
            i == 0 && j == 0 ? 1 :
            i == 0 && j == 1 ? 3 :
            i == 1 && j == 0 ? 4
                             : 2;
    } else {
        //Siamese method
        return n * (((i+1) + (j+1) - 1 + n/2) % n) + (((i+1) + 2 * (j+1) - 2) % n) + 1;
    }
}

} //end of namespace detail

//Note: Matrix of even order > 2 are only pseudo-magic
//TODO Add algorithm for even order
template<typename V>
struct magic_view {
    using value_type = V;

    const std::size_t n;

    magic_view(std::size_t n) : n(n) {}

    value_type operator[](std::size_t i) const {
        return detail::compute<value_type>(n, i / n, i % n);
    }

    value_type operator[](std::size_t i){
        return detail::compute<value_type>(n, i / n, i % n);
    }

    value_type operator()(std::size_t i, std::size_t j){
        return detail::compute<value_type>(n, i, j);
    }

    value_type operator()(std::size_t i, std::size_t j) const {
        return detail::compute<value_type>(n, i, j);
    }
};

template<typename V, std::size_t N>
struct fast_magic_view {
    using value_type = V;

    value_type operator[](std::size_t i) const {
        return detail::compute<value_type>(N, i / N, i % N);
    }

    value_type operator[](std::size_t i){
        return detail::compute<value_type>(N, i / N, i % N);
    }

    value_type operator()(std::size_t i, std::size_t j){
        return detail::compute<value_type>(N, i, j);
    }

    value_type operator()(std::size_t i, std::size_t j) const {
        return detail::compute<value_type>(N, i, j);
    }
};

} //end of namespace etl

#endif