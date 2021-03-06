//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*!
 * \file
 * \brief Convolution implementations with NVidia cuDNN library
 */

#pragma once

#ifdef ETL_CUDNN_MODE

#include "etl/impl/cublas/cuda.hpp"
#include "etl/impl/cudnn/cudnn.hpp"

#endif

namespace etl {

namespace impl {

namespace cudnn {

#ifdef ETL_CUDNN_MODE

template<typename T>
struct cudnn_wrapper {
    T tensor;

    cudnn_wrapper(T tensor) : tensor(tensor) {}

    cudnn_wrapper(const cudnn_wrapper& rhs) = delete;
    cudnn_wrapper& operator=(const cudnn_wrapper& rhs) = delete;

    cudnn_wrapper(cudnn_wrapper&& rhs){
        tensor = rhs.tensor;
        rhs.tensor = nullptr;
    }

    cudnn_wrapper& operator=(cudnn_wrapper&& rhs) = delete;

    T operator*(){
        return tensor;
    }

    ~cudnn_wrapper();
};

template<>
inline cudnn_wrapper<cudnnTensorDescriptor_t>::~cudnn_wrapper(){
    if(tensor){
        cudnn_check(cudnnDestroyTensorDescriptor(tensor));
    }
}

template<>
inline cudnn_wrapper<cudnnFilterDescriptor_t>::~cudnn_wrapper(){
    if(tensor){
        cudnn_check(cudnnDestroyFilterDescriptor(tensor));
    }
}

template<typename T>
cudnn_wrapper<cudnnTensorDescriptor_t> create_tensor(const opaque_memory<T,2>& input){
    auto data_type = std::is_same<std::remove_const_t<T>, float>::value ? CUDNN_DATA_FLOAT : CUDNN_DATA_DOUBLE;

    cudnnTensorDescriptor_t tensor;
    cudnn_check(cudnnCreateTensorDescriptor(&tensor));
    cudnn_check(cudnnSetTensor4dDescriptor(tensor, CUDNN_TENSOR_NCHW, data_type, 1, 1,
        input.template dim<0>(), input.template dim<1>()));

    return cudnn_wrapper<cudnnTensorDescriptor_t>{tensor};
}

template<typename T>
cudnn_wrapper<cudnnTensorDescriptor_t> create_tensor(const opaque_memory<T, 4>& input){
    auto data_type = std::is_same<std::remove_const_t<T>, float>::value ? CUDNN_DATA_FLOAT : CUDNN_DATA_DOUBLE;

    cudnnTensorDescriptor_t tensor;
    cudnn_check(cudnnCreateTensorDescriptor(&tensor));
    cudnn_check(cudnnSetTensor4dDescriptor(tensor, CUDNN_TENSOR_NCHW, data_type,
        input.template dim<0>(), input.template dim<1>(), input.template dim<2>(), input.template dim<3>()));

    return cudnn_wrapper<cudnnTensorDescriptor_t>{tensor};
}

template<typename T>
cudnn_wrapper<cudnnFilterDescriptor_t> create_filter(const opaque_memory<T,2>& kernel){
    auto data_type = std::is_same<std::remove_const_t<T>, float>::value ? CUDNN_DATA_FLOAT : CUDNN_DATA_DOUBLE;

    cudnnFilterDescriptor_t filter;
    cudnn_check(cudnnCreateFilterDescriptor(&filter));
    cudnn_check(cudnnSetFilter4dDescriptor(filter, data_type, CUDNN_TENSOR_NCHW,
        1, 1, kernel.template dim<0>(), kernel.template dim<1>()));

    return cudnn_wrapper<cudnnFilterDescriptor_t>{filter};
}

template<typename T>
cudnn_wrapper<cudnnFilterDescriptor_t> create_filter(const opaque_memory<T, 4>& kernel){
    auto data_type = std::is_same<std::remove_const_t<T>, float>::value ? CUDNN_DATA_FLOAT : CUDNN_DATA_DOUBLE;

    cudnnFilterDescriptor_t filter;
    cudnn_check(cudnnCreateFilterDescriptor(&filter));
    cudnn_check(cudnnSetFilter4dDescriptor(filter, data_type, CUDNN_TENSOR_NCHW,
        kernel.template dim<0>(), kernel.template dim<1>(), kernel.template dim<2>(), kernel.template dim<3>()));

    return cudnn_wrapper<cudnnFilterDescriptor_t>{filter};
}

/*!
 * \brief CUDNN implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename T>
void conv2_valid_set(const opaque_memory<T,2>& input, const opaque_memory<T,2>& kernel, const opaque_memory<T,2>& conv, size_t s1, size_t s2, size_t p1, size_t p2, cudnnConvolutionMode_t mode) {
    using type = std::remove_const_t<T>;

    type alpha[] = {1.0f};
    type beta[] = {0.0f};

    decltype(auto) handle = start_cudnn();

    // Prepare the tensors
    auto input_tensor  = create_tensor(input);
    auto output_tensor = create_tensor(conv);
    auto filter        = create_filter(kernel);

    // Prepare the convolution
    cudnnConvolutionDescriptor_t convolution;
    cudnn_check(cudnnCreateConvolutionDescriptor(&convolution));
    cudnn_check(cudnnSetConvolution2dDescriptor(convolution, p1, p2, s1, s2, 1, 1, mode));

    // Find the algorithm to use
    cudnnConvolutionFwdAlgo_t conv_algo;
    cudnn_check(cudnnGetConvolutionForwardAlgorithm(handle.get(), *input_tensor, *filter, convolution,
        *output_tensor, CUDNN_CONVOLUTION_FWD_SPECIFY_WORKSPACE_LIMIT, cudnn_max_workspace, &conv_algo));

    // Prepare the workspace
    std::size_t workspace_size = 0;
    cudnn_check(cudnnGetConvolutionForwardWorkspaceSize(handle.get(), *input_tensor, *filter, convolution, *output_tensor, conv_algo, &workspace_size));

    impl::cuda::cuda_memory<type> workspace;

    if(workspace_size){
        workspace = impl::cuda::cuda_allocate_only<type>(workspace_size);
    }

    // Allocate GPU memory, if necessary

    input.gpu_allocate_copy_if_necessary();
    kernel.gpu_allocate_copy_if_necessary();
    conv.gpu_allocate_if_necessary();

    // Perform the convolution

    cudnn_check(cudnnConvolutionForward(handle.get(),
        alpha, *input_tensor, input.gpu_memory(),
        *filter, kernel.gpu_memory(),
        convolution, conv_algo, workspace.get(), workspace_size,
        beta, *output_tensor, conv.gpu_memory()));

    // Release the resources
    cudnn_check(cudnnDestroyConvolutionDescriptor(convolution));
}

/*!
 * \brief CUDNN implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename T>
void conv2_valid(const opaque_memory<T,2>& input, const opaque_memory<T,2>& kernel, const opaque_memory<T,2>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    conv2_valid_set(input, kernel, conv, s1, s2, p1, p2, CUDNN_CONVOLUTION);
}

template <typename T>
void conv2_valid_flipped(const opaque_memory<T,2>& input, const opaque_memory<T,2>& kernel, const opaque_memory<T,2>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    conv2_valid_set(input, kernel, conv, s1, s2, p1, p2, CUDNN_CROSS_CORRELATION);
}

/*!
 * \brief cudnn implementation of a 4D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_set(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv, size_t s1, size_t s2, size_t p1, size_t p2, cudnnConvolutionMode_t mode) {
    using type = T;

    type alpha[] = {1.0f};
    type beta[] = {0.0f};

    decltype(auto) handle = start_cudnn();

    // Prepare the tensors
    auto input_tensor  = create_tensor(input);
    auto output_tensor = create_tensor(conv);
    auto filter        = create_filter(kernel);

    // Prepare the convolution
    cudnnConvolutionDescriptor_t convolution;
    cudnn_check(cudnnCreateConvolutionDescriptor(&convolution));
    cudnn_check(cudnnSetConvolution2dDescriptor(convolution, p1, p2, s1, s2, 1, 1, mode));

    // Find the algorithm to use
    cudnnConvolutionFwdAlgo_t conv_algo;
    cudnn_check(cudnnGetConvolutionForwardAlgorithm(handle.get(), *input_tensor, *filter, convolution,
        *output_tensor, CUDNN_CONVOLUTION_FWD_SPECIFY_WORKSPACE_LIMIT, cudnn_max_workspace, &conv_algo));

    // Prepare the workspace
    std::size_t workspace_size = 0;
    cudnn_check(cudnnGetConvolutionForwardWorkspaceSize(handle.get(), *input_tensor, *filter, convolution, *output_tensor, conv_algo, &workspace_size));

    impl::cuda::cuda_memory<type> workspace;

    if(workspace_size){
        workspace = impl::cuda::cuda_allocate_only<type>(workspace_size);
    }

    // Allocate GPU memory, if necessary

    input.gpu_allocate_copy_if_necessary();
    kernel.gpu_allocate_copy_if_necessary();
    conv.gpu_allocate_if_necessary();

    // Perform the convolution

    cudnn_check(cudnnConvolutionForward(handle.get(),
        alpha, *input_tensor, input.gpu_memory(),
        *filter, kernel.gpu_memory(),
        convolution, conv_algo, workspace.get(), workspace_size,
        beta, *output_tensor, conv.gpu_memory()));

    // Release the resources
    cudnn_check(cudnnDestroyConvolutionDescriptor(convolution));
}

/*!
 * \brief cudnn implementation of a 4D 'valid' convolution C = I * K, with flipped weights
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    conv4_valid_set(input, kernel, conv, s1, s2, p1, p2, CUDNN_CONVOLUTION);
}

template <typename T>
void conv4_valid_flipped(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    conv4_valid_set(input, kernel, conv, s1, s2, p1, p2, CUDNN_CROSS_CORRELATION);
}

/*!
 * \brief AVX implementation of a 4D 'valid' convolution C = I * K, where the output
 * are considered to be kernels
 *
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_filter_set(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv, size_t s1, size_t s2, size_t p1, size_t p2, cudnnConvolutionMode_t mode) {
    using type = T;

    type alpha[] = {1.0f};
    type beta[] = {0.0f};

    decltype(auto) handle = start_cudnn();

    // Prepare the tensors
    auto input_tensor  = create_tensor(input);
    auto output_tensor = create_tensor(kernel);
    auto filter        = create_filter(conv);

    // Prepare the convolution
    cudnnConvolutionDescriptor_t convolution;
    cudnn_check(cudnnCreateConvolutionDescriptor(&convolution));
    cudnn_check(cudnnSetConvolution2dDescriptor(convolution, p1, p2, s1, s2, 1, 1, mode));

    // Find the algorithm to use
    cudnnConvolutionBwdFilterAlgo_t conv_algo;
    cudnn_check(cudnnGetConvolutionBackwardFilterAlgorithm(handle.get(), *input_tensor, *output_tensor, convolution,
        *filter, CUDNN_CONVOLUTION_BWD_FILTER_SPECIFY_WORKSPACE_LIMIT, cudnn_max_workspace, &conv_algo));

    // Prepare the workspace
    std::size_t workspace_size = 0;
    cudnn_check(cudnnGetConvolutionBackwardFilterWorkspaceSize(handle.get(), *input_tensor, *output_tensor, convolution, *filter, conv_algo, &workspace_size));

    impl::cuda::cuda_memory<type> workspace;

    if(workspace_size){
        workspace = impl::cuda::cuda_allocate_only<type>(workspace_size);
    }

    // Allocate GPU memory, if necessary

    input.gpu_allocate_copy_if_necessary();
    kernel.gpu_allocate_copy_if_necessary();
    conv.gpu_allocate_if_necessary();

    // Perform the convolution

    cudnn_check(cudnnConvolutionBackwardFilter(handle.get(),
        alpha, *input_tensor, input.gpu_memory(),
        *output_tensor, kernel.gpu_memory(),
        convolution, conv_algo, workspace.get(), workspace_size,
        beta, *filter, conv.gpu_memory()));

    // Release the resources
    cudnn_check(cudnnDestroyConvolutionDescriptor(convolution));
}

/*!
 * \brief AVX implementation of a 4D 'valid' convolution C = I * K, where the output
 * are considered to be kernels, with flipped weights
 *
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_filter(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    conv4_valid_filter_set(input, kernel, conv, s1, s2, p1, p2, CUDNN_CONVOLUTION);
}

template <typename T>
void conv4_valid_filter_flipped(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    conv4_valid_filter_set(input, kernel, conv, s1, s2, p1, p2, CUDNN_CROSS_CORRELATION);
}

/*!
 * \brief cudnn implementation of a 2D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_full_set(const opaque_memory<T,2>& input, const opaque_memory<T,2>& kernel, const opaque_memory<T,2>& conv, cudnnConvolutionMode_t mode) {
    using type = std::remove_const_t<T>;

    type alpha[] = {1.0f};
    type beta[] = {0.0f};

    decltype(auto) handle = start_cudnn();

    // Prepare the tensors
    auto input_tensor  = create_tensor(input);
    auto output_tensor = create_tensor(conv);
    auto filter        = create_filter(kernel);

    // Prepare the convolution
    cudnnConvolutionDescriptor_t convolution;
    cudnn_check(cudnnCreateConvolutionDescriptor(&convolution));
    cudnn_check(cudnnSetConvolution2dDescriptor(convolution, 0, 0, 1, 1, 1, 1, mode));

    // Find the algorithm to use
    cudnnConvolutionBwdDataAlgo_t conv_algo;
    cudnn_check(cudnnGetConvolutionBackwardDataAlgorithm(handle.get(), *filter, *input_tensor, convolution,
        *output_tensor, CUDNN_CONVOLUTION_BWD_DATA_SPECIFY_WORKSPACE_LIMIT, cudnn_max_workspace, &conv_algo));

    // Prepare the workspace
    std::size_t workspace_size = 0;
    cudnn_check(cudnnGetConvolutionBackwardDataWorkspaceSize(handle.get(), *filter, *input_tensor, convolution, *output_tensor, conv_algo, &workspace_size));

    impl::cuda::cuda_memory<type> workspace;

    if(workspace_size){
        workspace = impl::cuda::cuda_allocate_only<type>(workspace_size);
    }

    // Allocate GPU memory, if necessary

    input.gpu_allocate_copy_if_necessary();
    kernel.gpu_allocate_copy_if_necessary();
    conv.gpu_allocate_if_necessary();

    // Perform the convolution

    cudnn_check(cudnnConvolutionBackwardData(handle.get(),
        alpha, *filter, kernel.gpu_memory(),
        *input_tensor, input.gpu_memory(),
        convolution, conv_algo, workspace.get(), workspace_size,
        beta, *output_tensor, conv.gpu_memory()));

    // Release the resources
    cudnn_check(cudnnDestroyConvolutionDescriptor(convolution));
}

/*!
 * \brief cudnn implementation of a 2D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_full(const opaque_memory<T,2>& input, const opaque_memory<T,2>& kernel, const opaque_memory<T,2>& conv) {
    conv2_full_set(input, kernel, conv, CUDNN_CROSS_CORRELATION);
}

template <typename T>
void conv2_full_flipped(const opaque_memory<T,2>& input, const opaque_memory<T,2>& kernel, const opaque_memory<T,2>& conv) {
    conv2_full_set(input, kernel, conv, CUDNN_CONVOLUTION);
}

/*!
 * \brief cudnn implementation of a 4D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_full_set(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv, cudnnConvolutionMode_t mode) {
    using type = T;

    type alpha[] = {1.0f};
    type beta[] = {0.0f};

    decltype(auto) handle = start_cudnn();

    // Prepare the tensors
    auto input_tensor  = create_tensor(input);
    auto output_tensor = create_tensor(conv);
    auto filter        = create_filter(kernel);

    // Prepare the convolution
    cudnnConvolutionDescriptor_t convolution;
    cudnn_check(cudnnCreateConvolutionDescriptor(&convolution));
    cudnn_check(cudnnSetConvolution2dDescriptor(convolution, 0, 0, 1, 1, 1, 1, mode));

    // Find the algorithm to use
    cudnnConvolutionBwdDataAlgo_t conv_algo;
    cudnn_check(cudnnGetConvolutionBackwardDataAlgorithm(handle.get(), *filter, *input_tensor, convolution,
        *output_tensor, CUDNN_CONVOLUTION_BWD_DATA_SPECIFY_WORKSPACE_LIMIT, cudnn_max_workspace, &conv_algo));

    // Prepare the workspace
    std::size_t workspace_size = 0;
    cudnn_check(cudnnGetConvolutionBackwardDataWorkspaceSize(handle.get(), *filter, *input_tensor, convolution, *output_tensor, conv_algo, &workspace_size));

    impl::cuda::cuda_memory<type> workspace;

    if(workspace_size){
        workspace = impl::cuda::cuda_allocate_only<type>(workspace_size);
    }

    // Allocate GPU memory, if necessary

    input.gpu_allocate_copy_if_necessary();
    kernel.gpu_allocate_copy_if_necessary();
    conv.gpu_allocate_if_necessary();

    // Perform the convolution

    cudnn_check(cudnnConvolutionBackwardData(handle.get(),
        alpha, *filter, kernel.gpu_memory(),
        *input_tensor, input.gpu_memory(),
        convolution, conv_algo, workspace.get(), workspace_size,
        beta, *output_tensor, conv.gpu_memory()));

    // Release the resources
    cudnn_check(cudnnDestroyConvolutionDescriptor(convolution));
}

/*!
 * \brief cudnn implementation of a 2D 'valid' convolution C = I * K, with multiple kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_full(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv) {
    conv4_full_set(input, kernel, conv, CUDNN_CROSS_CORRELATION);
}

template <typename T>
void conv4_full_flipped(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv) {
    conv4_full_set(input, kernel, conv, CUDNN_CONVOLUTION);
}

/*!
 * \brief CUDNN implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_valid_multi_set(const opaque_memory<T,2>& input, const opaque_memory<T,3>& kernel, const opaque_memory<T,3>& conv, size_t s1, size_t s2, size_t p1, size_t p2, cudnnConvolutionMode_t mode) {
    using type = std::remove_const_t<T>;

    auto data_type = std::is_same<type, float>::value ? CUDNN_DATA_FLOAT : CUDNN_DATA_DOUBLE;

    type alpha[] = {1.0f};
    type beta[] = {0.0f};

    decltype(auto) handle = start_cudnn();

    // Prepare the input tensor
    cudnnTensorDescriptor_t input_tensor;
    cudnn_check(cudnnCreateTensorDescriptor(&input_tensor));
    cudnn_check(cudnnSetTensor4dDescriptor(input_tensor, CUDNN_TENSOR_NCHW, data_type, 1, 1, input.template dim<0>(), input.template dim<1>()));

    // Prepare the output tensor
    cudnnTensorDescriptor_t output_tensor;
    cudnn_check(cudnnCreateTensorDescriptor(&output_tensor));
    cudnn_check(cudnnSetTensor4dDescriptor(output_tensor, CUDNN_TENSOR_NCHW, data_type, 1, conv.template dim<0>(), conv.template dim<1>(), conv.template dim<2>()));

    // Prepare the filter
    cudnnFilterDescriptor_t filter;
    cudnn_check(cudnnCreateFilterDescriptor(&filter));
    cudnn_check(cudnnSetFilter4dDescriptor(filter, data_type, CUDNN_TENSOR_NCHW, kernel.template dim<0>(), 1, kernel.template dim<1>(), kernel.template dim<2>()));

    // Prepare the convolution
    cudnnConvolutionDescriptor_t convolution;
    cudnn_check(cudnnCreateConvolutionDescriptor(&convolution));
    cudnn_check(cudnnSetConvolution2dDescriptor(convolution, p1, p2, s1, s2, 1, 1, mode));

    // Find the algorithm to use
    cudnnConvolutionFwdAlgo_t conv_algo;
    cudnn_check(cudnnGetConvolutionForwardAlgorithm(handle.get(), input_tensor, filter, convolution,
        output_tensor, CUDNN_CONVOLUTION_FWD_SPECIFY_WORKSPACE_LIMIT, cudnn_max_workspace, &conv_algo));

    // Prepare the workspace
    std::size_t workspace_size = 0;
    cudnn_check(cudnnGetConvolutionForwardWorkspaceSize(handle.get(), input_tensor, filter, convolution, output_tensor, conv_algo, &workspace_size));

    impl::cuda::cuda_memory<type> workspace;

    if(workspace_size){
        workspace = impl::cuda::cuda_allocate_only<type>(workspace_size);
    }

    // Allocate GPU memory, if necessary

    input.gpu_allocate_copy_if_necessary();
    kernel.gpu_allocate_copy_if_necessary();
    conv.gpu_allocate_if_necessary();

    // Perform the convolution

    cudnn_check(cudnnConvolutionForward(handle.get(),
        alpha, input_tensor, input.gpu_memory(),
        filter, kernel.gpu_memory(),
        convolution, conv_algo, workspace.get(), workspace_size,
        beta, output_tensor, conv.gpu_memory()));

    // Release the resources
    cudnn_check(cudnnDestroyConvolutionDescriptor(convolution));
    cudnn_check(cudnnDestroyFilterDescriptor(filter));
    cudnn_check(cudnnDestroyTensorDescriptor(output_tensor));
    cudnn_check(cudnnDestroyTensorDescriptor(input_tensor));
}

/*!
 * \brief Standard implementation of a 2D 'valid' convolution C = I * K, with multiple flipped kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_valid_multi(const opaque_memory<T,2>& input, const opaque_memory<T,3>& kernel, const opaque_memory<T,3>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    conv2_valid_multi_set(input, kernel, conv, s1, s2, p1, p2, CUDNN_CONVOLUTION);
}

template <typename T>
void conv2_valid_multi_flipped(const opaque_memory<T,2>& input, const opaque_memory<T,3>& kernel, const opaque_memory<T,3>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    conv2_valid_multi_set(input, kernel, conv, s1, s2, p1, p2, CUDNN_CROSS_CORRELATION);
}

//TODO Make real cudnn calls for the following two functions
//For some reason, it is not possible for backward convolution to expand the
//number of feature maps, therefore, it is necessary to make many calls to sub
//routines, which is highly inefficient and will result in many GPU allocations

/*!
 * \brief Standard implementation of a 2D 'full' convolution C = I * K, with multiple kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_full_multi(const I& input, const K& kernel, C&& conv) {
    auto input_gpu = input.direct();

    for(std::size_t i = 0; i < kernel.template dim<0>(); ++i){
        decltype(auto) result = conv(i);
        auto result_gpu = result.direct();

        conv2_full(input_gpu, kernel(i).direct(), result_gpu);
        result_gpu.gpu_copy_from();
        result_gpu.gpu_evict();
    }
}

/*!
 * \brief Standard implementation of a 2D 'full' convolution C = I * K, with multiple kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_full_multi_flipped(const I& input, const K& kernel, C&& conv) {
    auto input_gpu = input.direct();

    for(std::size_t i = 0; i < kernel.template dim<0>(); ++i){
        decltype(auto) result = conv(i);
        auto result_gpu = result.direct();

        conv2_full_flipped(input_gpu, kernel(i).direct(), result_gpu);
        result_gpu.gpu_copy_from();
        result_gpu.gpu_evict();
    }
}

template <typename I, typename K, typename C>
void conv2_full_multi_real_set(const I& input, const K& kernel, C&& conv, cudnnConvolutionMode_t mode) {
    using type = value_t<I>;

    auto data_type = std::is_same<type, float>::value ? CUDNN_DATA_FLOAT : CUDNN_DATA_DOUBLE;

    type alpha[] = {1.0f};
    type beta[] = {0.0f};

    decltype(auto) handle = start_cudnn();

    // Prepare the input tensor
    cudnnTensorDescriptor_t input_tensor;
    cudnn_check(cudnnCreateTensorDescriptor(&input_tensor));
    cudnn_check(cudnnSetTensor4dDescriptor(input_tensor, CUDNN_TENSOR_NCHW, data_type, 1, 1, input.template dim<0>(), input.template dim<1>()));

    // Prepare the output tensor
    cudnnTensorDescriptor_t output_tensor;
    cudnn_check(cudnnCreateTensorDescriptor(&output_tensor));
    cudnn_check(cudnnSetTensor4dDescriptor(output_tensor, CUDNN_TENSOR_NCHW, data_type, 1, conv.template dim<0>(), conv.template dim<1>(), conv.template dim<2>()));

    // Prepare the filter
    cudnnFilterDescriptor_t filter;
    cudnn_check(cudnnCreateFilterDescriptor(&filter));
    cudnn_check(cudnnSetFilter4dDescriptor(filter, data_type, CUDNN_TENSOR_NCHW, kernel.template dim<0>(), 1, kernel.template dim<1>(), kernel.template dim<2>()));

    // Prepare the convolution
    cudnnConvolutionDescriptor_t convolution;
    cudnn_check(cudnnCreateConvolutionDescriptor(&convolution));
    cudnn_check(cudnnSetConvolution2dDescriptor(convolution, 0, 0, 1, 1, 1, 1, mode));

    // Find the algorithm to use
    cudnnConvolutionBwdDataAlgo_t conv_algo;
    cudnn_check(cudnnGetConvolutionBackwardDataAlgorithm(handle.get(), filter, input_tensor, convolution,
        output_tensor, CUDNN_CONVOLUTION_BWD_DATA_SPECIFY_WORKSPACE_LIMIT, cudnn_max_workspace, &conv_algo));

    // Prepare the workspace
    std::size_t workspace_size = 0;
    cudnn_check(cudnnGetConvolutionBackwardDataWorkspaceSize(handle.get(), filter, input_tensor, convolution, output_tensor, conv_algo, &workspace_size));

    impl::cuda::cuda_memory<type> workspace;

    if(workspace_size){
        workspace = impl::cuda::cuda_allocate_only<type>(workspace_size);
    }

    // Allocate GPU memory, if necessary

    auto input_gpu = input.direct();
    auto kernel_gpu = kernel.direct();
    auto conv_gpu = conv.direct();

    input_gpu.gpu_allocate_copy_if_necessary();
    kernel_gpu.gpu_allocate_copy_if_necessary();
    conv_gpu.gpu_allocate_if_necessary();

    // Perform the convolution

    cudnn_check(cudnnConvolutionBackwardData(handle.get(),
        alpha, filter, kernel_gpu.gpu_memory(),
        input_tensor, input_gpu.gpu_memory(),
        convolution, conv_algo, workspace.get(), workspace_size,
        beta, output_tensor, conv_gpu.gpu_memory()));

    // Release the resources
    cudnn_check(cudnnDestroyConvolutionDescriptor(convolution));
    cudnn_check(cudnnDestroyFilterDescriptor(filter));
    cudnn_check(cudnnDestroyTensorDescriptor(output_tensor));
    cudnn_check(cudnnDestroyTensorDescriptor(input_tensor));
}

template <typename I, typename K, typename C>
void conv2_full_multi_real(const I& input, const K& kernel, C&& conv) {
    conv2_full_multi_real_set(input, kernel, conv, CUDNN_CROSS_CORRELATION);
}

template <typename I, typename K, typename C>
void conv2_full_multi_flipped_real(const I& input, const K& kernel, C&& conv) {
    conv2_full_multi_real_set(input, kernel, conv, CUDNN_CONVOLUTION);
}

#else

//COVERAGE_EXCLUDE_BEGIN

/*!
 * \brief CUDNN implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename T>
void conv2_valid(const opaque_memory<T,2>& input, const opaque_memory<T,2>& kernel, const opaque_memory<T,2>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("AVX not available/enabled");
}

/*!
 * \brief CUDNN implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename T>
void conv2_valid_flipped(const opaque_memory<T,2>& input, const opaque_memory<T,2>& kernel, const opaque_memory<T,2>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("AVX not available/enabled");
}

/*!
 * \brief cudnn implementation of a 4D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("Unsupported feature called: cudnn conv4_valid");
}

/*!
 * \brief cudnn implementation of a 4D 'valid' convolution C = I * K, with flipped weights
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_flipped(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("Unsupported feature called: cudnn conv4_valid_flipped");
}

/*!
 * \brief AVX implementation of a 4D 'valid' convolution C = I * K, where the output
 * are considered to be kernels
 *
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_filter(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("Unsupported feature called: cudnn conv4_valid_filter");
}

/*!
 * \brief AVX implementation of a 4D 'valid' convolution C = I * K, where the output
 * are considered to be kernels, with flipped weights
 *
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_filter_flipped(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("Unsupported feature called: cudnn conv4_valid_filter_flipped");
}

/*!
 * \brief cudnn implementation of a 2D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_full(const opaque_memory<T,2>& input, const opaque_memory<T,2>& kernel, const opaque_memory<T,2>& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("Unsupported feature called: cudnn conv2_full");
}

/*!
 * \brief cudnn implementation of a 2D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_full_flipped(const opaque_memory<T,2>& input, const opaque_memory<T,2>& kernel, const opaque_memory<T,2>& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("Unsupported feature called: cudnn conv2_full_flipped");
}

/*!
 * \brief cudnn implementation of a 4D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_full(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("Unsupported feature called: cudnn conv4_full");
}

/*!
 * \brief cudnn implementation of a 2D 'valid' convolution C = I * K, with multiple kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_full_flipped(const opaque_memory<T,4>& input, const opaque_memory<T,4>& kernel, const opaque_memory<T,4>& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("Unsupported feature called: cudnn conv4_ful_flippedl");
}

/*!
 * \brief CUDNN implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_valid_multi(const opaque_memory<T,2>& input, const opaque_memory<T,3>& kernel, const opaque_memory<T,3>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("Unsupported feature called: cudnn conv2_valid_multi");
}

/*!
 * \brief Standard implementation of a 2D 'valid' convolution C = I * K, with multiple flipped kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_valid_multi_flipped(const opaque_memory<T,2>& input, const opaque_memory<T,3>& kernel, const opaque_memory<T,3>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("Unsupported feature called: cudnn conv2_valid_multi_flipped");
}

/*!
 * \brief Standard implementation of a 2D 'full' convolution C = I * K, with multiple kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_full_multi(const I& input, const K& kernel, C&& conv){
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("Unsupported feature called: cudnn conv2_full_multi");
}

/*!
 * \brief Standard implementation of a 2D 'full' convolution C = I * K, with multiple kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_full_multi_flipped(const I& input, const K& kernel, C&& conv){
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("Unsupported feature called: cudnn conv2_full_multi_flipped");
}

//COVERAGE_EXCLUDE_END

#endif

} //end of namespace cudnn

} //end of namespace impl

} //end of namespace etl
