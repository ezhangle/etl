//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*!
 * \file
 * \brief Contains thresholds to select implementations based
 * on the expression size
 */

#pragma once

namespace etl {

constexpr std::size_t gemm_std_max    = 75 * 75;   ///< The maximum number of elements to be handled by std algorithm
constexpr std::size_t gemm_cublas_min = 180 * 180; ///< The minimum number or elements before considering cublas

constexpr std::size_t gevm_small_threshold = 62000;   ///< The number of elements of b after which we use BLAS-like kernel
constexpr std::size_t gemv_small_threshold = 4500000; ///< The number of elements of A after which we use BLAS-like kernel

constexpr std::size_t parallel_threshold = 128 * 1024; ///< The minimum number of elements before considering parallel implementation

constexpr std::size_t sum_parallel_threshold = 1024 * 32; ///< The minimum number of elements before considering parallel acc implementation

constexpr std::size_t conv1_parallel_threshold_conv   = 100; ///< The mimum output size before considering parallel convolution
constexpr std::size_t conv1_parallel_threshold_kernel = 16;  ///< The mimum kernel size before considering parallel convolution

constexpr std::size_t fft1_many_threshold_transforms = 16;  ///< The mimum number of transforms to parallelize them
constexpr std::size_t fft1_many_threshold_n          = 768; ///< The mimum size of the transforms to parallelize them

constexpr std::size_t fft2_many_threshold_transforms = 16;   ///< The mimum number of transforms to parallelize them
constexpr std::size_t fft2_many_threshold_n          = 1024; ///< The mimum size of the transforms to parallelize them

} //end of namespace etl
