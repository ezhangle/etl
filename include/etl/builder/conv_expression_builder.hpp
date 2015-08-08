//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*!
 * \file conv_expression_builder.hpp
 * \brief Contains all the operators and functions to build convolution expressions.
*/

#pragma once

#include "etl/config.hpp"
#include "etl/expression_helpers.hpp"

namespace etl {

template<typename A, typename B>
auto conv_1d_valid(A&& a, B&& b) -> detail::temporary_binary_helper<A, B, conv1_valid_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value, "Convolution only supported for ETL expressions");

    return {a, b};
}

template<typename A, typename B, typename C>
auto conv_1d_valid(A&& a, B&& b, C&& c) -> detail::forced_temporary_binary_helper<A, B, C, conv1_valid_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value && is_etl_expr<C>::value, "Convolution only supported for ETL expressions");

    return {a, b, c};
}

template<typename A, typename B>
auto conv_1d_same(A&& a, B&& b) -> detail::temporary_binary_helper<A, B, conv1_same_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value, "Convolution only supported for ETL expressions");

    return {a, b};
}

template<typename A, typename B, typename C>
auto conv_1d_same(A&& a, B&& b, C&& c) -> detail::forced_temporary_binary_helper<A, B, C, conv1_same_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value && is_etl_expr<C>::value, "Convolution only supported for ETL expressions");

    return {a, b, c};
}

template<typename A, typename B>
auto conv_1d_full(A&& a, B&& b) -> detail::temporary_binary_helper<A, B, conv1_full_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value, "Convolution only supported for ETL expressions");

    return {a, b};
}

template<typename A, typename B, typename C>
auto conv_1d_full(A&& a, B&& b, C&& c) -> detail::forced_temporary_binary_helper<A, B, C, conv1_full_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value && is_etl_expr<C>::value, "Convolution only supported for ETL expressions");

    return {a, b, c};
}

template<typename A, typename B>
auto fft_conv_1d_full(A&& a, B&& b) -> detail::temporary_binary_helper<A, B, fft_conv1_full_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value, "Convolution only supported for ETL expressions");

    return {a, b};
}

template<typename A, typename B, typename C>
auto fft_conv_1d_full(A&& a, B&& b, C&& c) -> detail::forced_temporary_binary_helper<A, B, C, fft_conv1_full_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value && is_etl_expr<C>::value, "Convolution only supported for ETL expressions");

    return {a, b, c};
}

template<typename A, typename B, cpp_enable_if_cst(has_fft::value)>
auto fast_conv_1d_full(A&& a, B&& b){
    return fft_conv_1d_full(std::forward<A>(a), std::forward<B>(b));
}

template<typename A, typename B, cpp_disable_if_cst(has_fft::value)>
auto fast_conv_1d_full(A&& a, B&& b){
    return conv_1d_full(std::forward<A>(a), std::forward<B>(b));
}

template<typename A, typename B, typename C, cpp_enable_if_cst(has_fft::value)>
auto fast_conv_1d_full(A&& a, B&& b, C&& c){
    return fft_conv_1d_full(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c));
}

template<typename A, typename B, typename C, cpp_disable_if_cst(has_fft::value)>
auto fast_conv_1d_full(A&& a, B&& b, C&& c){
    return conv_1d_full(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c));
}

template<typename A, typename B>
auto conv_2d_valid(A&& a, B&& b) -> detail::temporary_binary_helper<A, B, conv2_valid_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value, "Convolution only supported for ETL expressions");

    return {a, b};
}

template<typename A, typename B, typename C>
auto conv_2d_valid(A&& a, B&& b, C&& c) -> detail::forced_temporary_binary_helper<A, B, C, conv2_valid_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value && is_etl_expr<C>::value, "Convolution only supported for ETL expressions");

    return {a, b, c};
}

template<typename A, typename B>
auto conv_2d_same(A&& a, B&& b) -> detail::temporary_binary_helper<A, B, conv2_same_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value, "Convolution only supported for ETL expressions");

    return {a, b};
}

template<typename A, typename B, typename C>
auto conv_2d_same(A&& a, B&& b, C&& c) -> detail::forced_temporary_binary_helper<A, B, C, conv2_same_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value && is_etl_expr<C>::value, "Convolution only supported for ETL expressions");

    return {a, b, c};
}

template<typename A, typename B>
auto conv_2d_full(A&& a, B&& b) -> detail::temporary_binary_helper<A, B, conv2_full_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value, "Convolution only supported for ETL expressions");

    return {a, b};
}

template<typename A, typename B, typename C>
auto conv_2d_full(A&& a, B&& b, C&& c) -> detail::forced_temporary_binary_helper<A, B, C, conv2_full_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value && is_etl_expr<C>::value, "Convolution only supported for ETL expressions");

    return {a, b, c};
}

template<typename A, typename B>
auto fft_conv_2d_full(A&& a, B&& b) -> detail::temporary_binary_helper<A, B, fft_conv2_full_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value, "Convolution only supported for ETL expressions");

    return {a, b};
}

template<typename A, typename B, typename C>
auto fft_conv_2d_full(A&& a, B&& b, C&& c) -> detail::forced_temporary_binary_helper<A, B, C, fft_conv2_full_expr> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value && is_etl_expr<C>::value, "Convolution only supported for ETL expressions");

    return {a, b, c};
}

template<typename A, typename B, cpp_enable_if_cst(has_fft::value)>
auto fast_conv_2d_full(A&& a, B&& b){
    return fft_conv_2d_full(std::forward<A>(a), std::forward<B>(b));
}

template<typename A, typename B, cpp_disable_if_cst(has_fft::value)>
auto fast_conv_2d_full(A&& a, B&& b){
    return conv_2d_full(std::forward<A>(a), std::forward<B>(b));
}

template<typename A, typename B, typename C, cpp_enable_if_cst(has_fft::value)>
auto fast_conv_2d_full(A&& a, B&& b, C&& c){
    return fft_conv_2d_full(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c));
}

template<typename A, typename B, typename C, cpp_disable_if_cst(has_fft::value)>
auto fast_conv_2d_full(A&& a, B&& b, C&& c){
    return conv_2d_full(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c));
}

template<typename A, typename B>
auto conv_deep_valid(A&& a, B&& b) -> detail::dim_temporary_binary_helper<A, B, conv_deep_valid_expr, decay_traits<A>::dimensions()> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value, "Convolution only supported for ETL expressions");

    return {a, b};
}

template<typename A, typename B, typename C>
auto conv_deep_valid(A&& a, B&& b, C&& c) -> detail::dim_forced_temporary_binary_helper<A, B, C, conv_deep_valid_expr, decay_traits<A>::dimensions()> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value && is_etl_expr<C>::value, "Convolution only supported for ETL expressions");

    return {a, b, c};
}

template<typename A, typename B>
auto conv_deep_same(A&& a, B&& b) -> detail::dim_temporary_binary_helper<A, B, conv_deep_same_expr, decay_traits<A>::dimensions()> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value, "Convolution only supported for ETL expressions");

    return {a, b};
}

template<typename A, typename B, typename C>
auto conv_deep_same(A&& a, B&& b, C&& c) -> detail::dim_forced_temporary_binary_helper<A, B, C, conv_deep_same_expr, decay_traits<A>::dimensions()> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value && is_etl_expr<C>::value, "Convolution only supported for ETL expressions");

    return {a, b, c};
}

template<typename A, typename B>
auto conv_deep_full(A&& a, B&& b) -> detail::dim_temporary_binary_helper<A, B, conv_deep_full_expr, decay_traits<A>::dimensions()> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value, "Convolution only supported for ETL expressions");

    return {a, b};
}

template<typename A, typename B, typename C>
auto conv_deep_full(A&& a, B&& b, C&& c) -> detail::dim_forced_temporary_binary_helper<A, B, C, conv_deep_full_expr, decay_traits<A>::dimensions()> {
    static_assert(is_etl_expr<A>::value && is_etl_expr<B>::value && is_etl_expr<C>::value, "Convolution only supported for ETL expressions");

    return {a, b, c};
}

//Special convolutions

//TODO This should be moved
//TODO This should be adapted to an expression
//TODO For now, the fast version only works with square kernels

template<typename A, typename B, typename C>
void conv_2d_valid_multi(A&& input, B&& kernels, C&& features){
    //TODO Validate inputs

    //TODO This version of the implementation should only be used if very fast MMUL is available

    if(input.is_square() && kernels.is_sub_square()){
        const auto v1 = etl::dim<0>(input);
        const auto v2 = etl::dim<1>(input);
        const auto k1 = etl::dim<1>(kernels);
        const auto k2 = etl::dim<2>(kernels);

        etl::dyn_matrix<value_t<A>, 2> input_col(k1 * k2, (v1 - k1 + 1) * (v2 - k2 + 1));

        conv_2d_valid_multi(std::forward<A>(input), std::forward<B>(kernels), std::forward<C>(features), input_col);
    } else {
        //Standard version
        for(size_t k = 0; k < etl::dim<0>(kernels); ++k){
            features(k) = conv_2d_valid(input, kernels(k));
        }
    }
}

template<typename A, typename B, typename C, typename D>
void conv_2d_valid_multi(A&& input, B&& kernels, C&& features, D&& input_col){
    cpp_assert(input.is_square() && kernels.is_sub_square(), "Only implemented for square input and kernels");

    //TODO Validate inputs

    etl::dyn_matrix<value_t<B>, 3> prepared_k(etl::dim<0>(kernels), etl::dim<1>(kernels), etl::dim<2>(kernels));

    for(std::size_t i = 0; i < etl::dim<0>(kernels); ++i){
        prepared_k(i) = transpose(fflip(kernels(i)));
    }

    conv_2d_valid_multi_prepared(std::forward<A>(input), prepared_k, std::forward<C>(features), std::forward<D>(input_col));
}

template<typename A, typename B, typename C, typename D>
void conv_2d_valid_multi_prepared(A&& input, B&& kernels, C&& features, D&& input_col){
    cpp_assert(input.is_square() && kernels.is_sub_square(), "Only implemented for square input and kernels");

    //TODO Validate inputs

    const auto K  = etl::dim<0>(kernels);
    const auto k1 = etl::dim<1>(kernels);
    const auto k2 = etl::dim<2>(kernels);

    im2col_direct(input_col, input, k1, k2);

    *mul(
        etl::reshape(kernels, K, k1 * k2),
        input_col,
        etl::reshape(features, K, etl::dim<1>(features) * etl::dim<2>(features)));

    for(std::size_t k = 0; k < K; ++k){
        features(k).transpose_inplace();
    }
}

template<typename A>
auto convmtx(A&& a, std::size_t h) -> detail::stable_transform_helper<A, dyn_convmtx_transformer> {
    static_assert(is_etl_expr<A>::value, "Convolution matrices only supported for ETL expressions");
    static_assert(decay_traits<A>::dimensions() == 1, "Convolutional matrix only works in 1D");

    return detail::stable_transform_helper<A, dyn_convmtx_transformer>{dyn_convmtx_transformer<detail::build_type<A>>(a, h)};
}

template<typename A>
auto convmtx2(A&& a, std::size_t k1, std::size_t k2) -> detail::stable_transform_helper<A, dyn_convmtx2_transformer> {
    static_assert(is_etl_expr<A>::value, "Convolution matrices only supported for ETL expressions");
    static_assert(decay_traits<A>::dimensions() == 2, "Convolutional matrix only works in 2D");

    return detail::stable_transform_helper<A, dyn_convmtx2_transformer>{dyn_convmtx2_transformer<detail::build_type<A>>(a, k1, k2)};
}

template<std::size_t K1, std::size_t K2, typename A>
auto convmtx2_direct(A&& a) -> temporary_unary_expr<value_t<A>, detail::build_type<A>, direct_convmtx2_expr<value_t<A>, K1, K2>, void> {
    static_assert(is_etl_expr<A>::value, "Convolution matrices only supported for ETL expressions");
    static_assert(decay_traits<A>::dimensions() == 2, "Convolutional matrix only works in 2D");

    return temporary_unary_expr<value_t<A>, detail::build_type<A>, direct_convmtx2_expr<value_t<A>, K1, K2>, void>{a};
}

} //end of namespace etl