//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#ifdef ETL_MKL_MODE
#include "mkl_dfti.h"
#endif

namespace etl {

namespace impl {

namespace blas {

#ifdef ETL_MKL_MODE

namespace mkl_detail {

inline void fft_kernel(const std::complex<float>* in, std::size_t s, std::complex<float>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 1, s); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);         //Out of place FFT
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr, out);                        //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void fft_kernel(const std::complex<double>* in, std::size_t s, std::complex<double>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 1, s); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);         //Out of place FFT
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr, out);                        //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void fft_many_kernel(const std::complex<float>* in, std::size_t batch, std::size_t n, std::complex<float>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 1, n); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);         //Out of place FFT
    DftiSetValue(descriptor, DFTI_NUMBER_OF_TRANSFORMS, batch);         //Number of transforms
    DftiSetValue(descriptor, DFTI_INPUT_DISTANCE, n);                   //Input stride
    DftiSetValue(descriptor, DFTI_OUTPUT_DISTANCE, n);                  //Output stride
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr, out);                        //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void fft_many_kernel(const std::complex<double>* in, std::size_t batch, std::size_t n, std::complex<double>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 1, n); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);         //Out of place FFT
    DftiSetValue(descriptor, DFTI_NUMBER_OF_TRANSFORMS, batch);         //Number of transforms
    DftiSetValue(descriptor, DFTI_INPUT_DISTANCE, n);                   //Input stride
    DftiSetValue(descriptor, DFTI_OUTPUT_DISTANCE, n);                  //Output stride
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr, out);                        //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void inplace_fft_kernel(std::complex<float>* in, std::size_t s) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = static_cast<void*>(in);

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 1, s); //Specify size and precision
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr);                             //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void inplace_fft_kernel(std::complex<double>* in, std::size_t s) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = static_cast<void*>(in);

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 1, s); //Specify size and precision
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr, in);                         //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void ifft_kernel(const std::complex<float>* in, std::size_t s, std::complex<float>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 1, s); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);         //Out of place FFT
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0f / s);            //Scale down the output
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr, out);                       //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void ifft_kernel(const std::complex<double>* in, std::size_t s, std::complex<double>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 1, s); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);         //Out of place FFT
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0 / s);             //Scale down the output
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr, out);                       //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void ifft_many_kernel(const std::complex<float>* in, std::size_t batch, std::size_t s, std::complex<float>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 1, s); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);         //Out of place FFT
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0f / s);            //Scale down the output
    DftiSetValue(descriptor, DFTI_NUMBER_OF_TRANSFORMS, batch);         //Number of transforms
    DftiSetValue(descriptor, DFTI_INPUT_DISTANCE, s);                   //Input stride
    DftiSetValue(descriptor, DFTI_OUTPUT_DISTANCE, s);                  //Output stride
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr, out);                       //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void ifft_many_kernel(const std::complex<double>* in, std::size_t batch, std::size_t s, std::complex<double>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 1, s); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);         //Out of place FFT
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0 / s);             //Scale down the output
    DftiSetValue(descriptor, DFTI_NUMBER_OF_TRANSFORMS, batch);         //Number of transforms
    DftiSetValue(descriptor, DFTI_INPUT_DISTANCE, s);                   //Input stride
    DftiSetValue(descriptor, DFTI_OUTPUT_DISTANCE, s);                  //Output stride
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr, out);                       //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void inplace_ifft_kernel(std::complex<float>* in, std::size_t s) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = static_cast<void*>(in);

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 1, s); //Specify size and precision
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0f / s);            //Scale down the output
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr);                            //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void inplace_ifft_kernel(std::complex<double>* in, std::size_t s) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    void* in_ptr = static_cast<void*>(in);

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 1, s); //Specify size and precision
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0 / s);             //Scale down the output
    DftiCommitDescriptor(descriptor);                                   //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr);                            //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                    //Free the descriptor
}

inline void fft2_kernel(const std::complex<float>* in, std::size_t d1, std::size_t d2, std::complex<float>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);           //Out of place FFT
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr, out);                          //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

inline void fft2_kernel(const std::complex<double>* in, std::size_t d1, std::size_t d2, std::complex<double>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);           //Out of place FFT
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr, out);                          //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

inline void fft2_kernel(const etl::complex<float>* in, std::size_t d1, std::size_t d2, etl::complex<float>* out) {
    fft2_kernel(reinterpret_cast<const std::complex<float>*>(in), d1, d2, reinterpret_cast<std::complex<float>*>(out));
}

inline void fft2_kernel(const etl::complex<double>* in, std::size_t d1, std::size_t d2, etl::complex<double>* out) {
    fft2_kernel(reinterpret_cast<const std::complex<double>*>(in), d1, d2, reinterpret_cast<std::complex<double>*>(out));
}

inline void fft2_many_kernel(const std::complex<float>* in, std::size_t batch, std::size_t d1, std::size_t d2, std::complex<float>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);           //Out of place FFT
    DftiSetValue(descriptor, DFTI_NUMBER_OF_TRANSFORMS, batch);           //Number of transforms
    DftiSetValue(descriptor, DFTI_INPUT_DISTANCE, d1 * d2);               //Input stride
    DftiSetValue(descriptor, DFTI_OUTPUT_DISTANCE, d1 * d2);              //Output stride
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr, out);                          //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

inline void fft2_many_kernel(const std::complex<double>* in, std::size_t batch, std::size_t d1, std::size_t d2, std::complex<double>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);           //Out of place FFT
    DftiSetValue(descriptor, DFTI_NUMBER_OF_TRANSFORMS, batch);           //Number of transforms
    DftiSetValue(descriptor, DFTI_INPUT_DISTANCE, d1 * d2);               //Input stride
    DftiSetValue(descriptor, DFTI_OUTPUT_DISTANCE, d1 * d2);              //Output stride
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr, out);                          //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

inline void fft2_many_kernel(const etl::complex<float>* in, std::size_t batch, std::size_t d1, std::size_t d2, etl::complex<float>* out) {
    fft2_many_kernel(reinterpret_cast<const std::complex<float>*>(in), batch, d1, d2, reinterpret_cast<std::complex<float>*>(out));
}

inline void fft2_many_kernel(const etl::complex<double>* in, std::size_t batch, std::size_t d1, std::size_t d2, etl::complex<double>* out) {
    fft2_many_kernel(reinterpret_cast<const std::complex<double>*>(in), batch, d1, d2, reinterpret_cast<std::complex<double>*>(out));
}

inline void inplace_fft2_kernel(std::complex<float>* in, std::size_t d1, std::size_t d2) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr);                               //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

inline void inplace_fft2_kernel(std::complex<double>* in, std::size_t d1, std::size_t d2) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeForward(descriptor, in_ptr);                               //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

inline void ifft2_kernel(const std::complex<float>* in, std::size_t d1, std::size_t d2, std::complex<float>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);           //Out of place FFT
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0f / (d1 * d2));      //Scale down the output
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr, out);                         //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

inline void ifft2_kernel(const std::complex<double>* in, std::size_t d1, std::size_t d2, std::complex<double>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);           //Out of place FFT
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0 / (d1 * d2));       //Scale down the output
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr, out);                         //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

inline void ifft2_many_kernel(const std::complex<float>* in, std::size_t batch, std::size_t d1, std::size_t d2, std::complex<float>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);           //Out of place FFT
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0f / (d1 * d2));      //Scale down the output
    DftiSetValue(descriptor, DFTI_NUMBER_OF_TRANSFORMS, batch);           //Number of transforms
    DftiSetValue(descriptor, DFTI_INPUT_DISTANCE, d1 * d2);               //Input stride
    DftiSetValue(descriptor, DFTI_OUTPUT_DISTANCE, d1 * d2);              //Output stride
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr, out);                         //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

inline void ifft2_many_kernel(const std::complex<double>* in, std::size_t batch, std::size_t d1, std::size_t d2, std::complex<double>* out) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);           //Out of place FFT
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0 / (d1 * d2));       //Scale down the output
    DftiSetValue(descriptor, DFTI_NUMBER_OF_TRANSFORMS, batch);           //Number of transforms
    DftiSetValue(descriptor, DFTI_INPUT_DISTANCE, d1 * d2);               //Input stride
    DftiSetValue(descriptor, DFTI_OUTPUT_DISTANCE, d1 * d2);              //Output stride
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr, out);                         //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

inline void inplace_ifft2_kernel(std::complex<float>* in, std::size_t d1, std::size_t d2) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0f / (d1 * d2));      //Scale down the output
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr);                              //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

inline void inplace_ifft2_kernel(std::complex<double>* in, std::size_t d1, std::size_t d2) {
    DFTI_DESCRIPTOR_HANDLE descriptor;

    MKL_LONG dim[]{static_cast<long>(d1), static_cast<long>(d2)};

    void* in_ptr = const_cast<void*>(static_cast<const void*>(in));

    DftiCreateDescriptor(&descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 2, dim); //Specify size and precision
    DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0 / (d1 * d2));       //Scale down the output
    DftiCommitDescriptor(descriptor);                                     //Finalize the descriptor
    DftiComputeBackward(descriptor, in_ptr);                              //Compute the Forward FFT
    DftiFreeDescriptor(&descriptor);                                      //Free the descriptor
}

template <typename T>
void conv2_full_kernel(const T* a, std::size_t m1, std::size_t m2, const T* b, std::size_t n1, std::size_t n2, T* c, T beta) {
    const std::size_t s1 = m1 + n1 - 1;
    const std::size_t s2 = m2 + n2 - 1;
    const std::size_t size = s1 * s2;

    dyn_vector<etl::complex<T>> a_padded(size);
    dyn_vector<etl::complex<T>> b_padded(size);

    for (std::size_t i = 0; i < m1; ++i) {
        direct_copy_n(a + i * m2, a_padded.memory_start() + i * s2, m2);
    }

    for (std::size_t i = 0; i < n1; ++i) {
        direct_copy_n(b + i * n2, b_padded.memory_start() + i * s2, n2);
    }

    inplace_fft2_kernel(reinterpret_cast<std::complex<T>*>(a_padded.memory_start()), s1, s2);
    inplace_fft2_kernel(reinterpret_cast<std::complex<T>*>(b_padded.memory_start()), s1, s2);

    a_padded *= b_padded;

    inplace_ifft2_kernel(reinterpret_cast<std::complex<T>*>(a_padded.memory_start()), s1, s2);

    if (beta == T(0.0)) {
        for (std::size_t i = 0; i < size; ++i) {
            c[i] = a_padded[i].real;
        }
    } else {
        for (std::size_t i = 0; i < size; ++i) {
            c[i] = beta * c[i] + a_padded[i].real;
        }
    }
}

} //End of namespace mkl_detail

/*!
 * \brief Perform the 1D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
inline void fft1(const opaque_memory<float,1>& a, const opaque_memory<std::complex<float>,1>& c) {
    auto a_complex = allocate<std::complex<float>>(a.size());

    direct_copy(a.memory_start(), a.memory_end(), a_complex.get());

    mkl_detail::fft_kernel(a_complex.get(), a.size(), c.memory_start());
}

/*!
 * \brief Perform the 1D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
inline void fft1(const opaque_memory<double,1>& a, const opaque_memory<std::complex<double>,1>& c) {
    auto a_complex = allocate<std::complex<double>>(a.size());

    direct_copy(a.memory_start(), a.memory_end(), a_complex.get());

    mkl_detail::fft_kernel(a_complex.get(), a.size(), c.memory_start());
}

/*!
 * \brief Perform the 1D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename T>
void fft1(const opaque_memory<std::complex<T>,1>& a, const opaque_memory<std::complex<T>,1>& c) {
    mkl_detail::fft_kernel(a.memory_start(), a.size(), c.memory_start());
}

/*!
 * \brief Perform the 1D Inverse FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C>
void ifft1(A&& a, C&& c) {
    mkl_detail::ifft_kernel(a.memory_start(), etl::size(a), c.memory_start());
}

/*!
 * \brief Perform the 1D Inverse FFT on a and store the real part of the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C, cpp_enable_if(all_complex_single_precision<A>::value)>
void ifft1_real(A&& a, C&& c) {
    auto c_complex = allocate<std::complex<float>>(etl::size(a));

    mkl_detail::ifft_kernel(a.memory_start(), etl::size(a), c_complex.get());

    for (std::size_t i = 0; i < etl::size(a); ++i) {
        c[i] = c_complex[i].real();
    }
}

/*!
 * \brief Perform the 1D Inverse FFT on a and store the real part of the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C, cpp_enable_if(all_complex_double_precision<A>::value)>
void ifft1_real(A&& a, C&& c) {
    auto c_complex = allocate<std::complex<double>>(etl::size(a));

    mkl_detail::ifft_kernel(a.memory_start(), etl::size(a), c_complex.get());

    for (std::size_t i = 0; i < etl::size(a); ++i) {
        c[i] = c_complex[i].real();
    }
}

/*!
 * \brief Perform many 1D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 *
 * The first dimension of a and c are considered batch dimensions
 */
template<std::size_t N, typename T>
void fft1_many(const opaque_memory<T, N>& a, const opaque_memory<std::complex<T>, N>& c) {
    std::size_t n     = a.template dim<N - 1>(); //Size of the transform
    std::size_t batch = a.size() / n;            //Number of batch

    auto a_complex = allocate<std::complex<T>>(a.size());

    direct_copy(a.memory_start(), a.memory_end(), a_complex.get());

    mkl_detail::fft_many_kernel(a_complex.get(), batch, n, c.memory_start());
}

/*!
 * \brief Perform many 1D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 *
 * The first dimension of a and c are considered batch dimensions
 */
template<std::size_t N, typename T>
void fft1_many(const opaque_memory<std::complex<T>, N>& a, const opaque_memory<std::complex<T>, N>& c) {
    std::size_t n     = a.template dim<N - 1>(); //Size of the transform
    std::size_t batch = a.size() / n;            //Number of batch

    mkl_detail::fft_many_kernel(a.memory_start(), batch, n, c.memory_start());
}

/*!
 * \brief Perform many 1D Inverse FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 *
 * The first dimension of a and c are considered batch dimensions
 */
template <typename A, typename C>
void ifft1_many(A&& a, C&& c) {
    static constexpr std::size_t N = decay_traits<A>::dimensions();

    std::size_t n     = etl::dim<N - 1>(a); //Size of the transform
    std::size_t batch = etl::size(a) / n;   //Number of batch

    mkl_detail::ifft_many_kernel(a.memory_start(), batch, n, c.memory_start());
}

/*!
 * \brief Perform the 1D full convolution of a with b and store the result in c
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename A, typename B, typename C>
void conv1_full(A&& a, B&& b, C&& c) {
    using type = value_t<A>;

    const std::size_t m    = etl::size(a);
    const std::size_t n    = etl::size(b);
    const std::size_t size = m + n - 1;

    //Note: use of value_t to make the type dependent!
    dyn_vector<etl::complex<type>> a_padded(etl::size(c));
    dyn_vector<etl::complex<type>> b_padded(etl::size(c));

    direct_copy(a.memory_start(), a.memory_end(), a_padded.memory_start());
    direct_copy(b.memory_start(), b.memory_end(), b_padded.memory_start());

    mkl_detail::inplace_fft_kernel(reinterpret_cast<std::complex<type>*>(a_padded.memory_start()), size);
    mkl_detail::inplace_fft_kernel(reinterpret_cast<std::complex<type>*>(b_padded.memory_start()), size);

    a_padded *= b_padded;

    mkl_detail::inplace_ifft_kernel(reinterpret_cast<std::complex<type>*>(a_padded.memory_start()), size);

    for (std::size_t i = 0; i < size; ++i) {
        c[i] = a_padded[i].real;
    }
}

/*!
 * \brief Perform the 2D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C, cpp_enable_if(all_single_precision<A>::value)>
void fft2(A&& a, C&& c) {
    auto a_complex = allocate<std::complex<float>>(etl::size(a));

    direct_copy(a.memory_start(), a.memory_end(), a_complex.get());

    mkl_detail::fft2_kernel(a_complex.get(), etl::dim<0>(a), etl::dim<1>(a), c.memory_start());
}

/*!
 * \brief Perform the 2D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C, cpp_enable_if(all_double_precision<A>::value)>
void fft2(A&& a, C&& c) {
    auto a_complex = allocate<std::complex<double>>(etl::size(a));

    direct_copy(a.memory_start(), a.memory_end(), a_complex.get());

    mkl_detail::fft2_kernel(a_complex.get(), etl::dim<0>(a), etl::dim<1>(a), c.memory_start());
}

/*!
 * \brief Perform the 2D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C, cpp_enable_if(all_complex<A>::value)>
void fft2(A&& a, C&& c) {
    mkl_detail::fft2_kernel(a.memory_start(), etl::dim<0>(a), etl::dim<1>(a), c.memory_start());
}

/*!
 * \brief Perform many 2D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 *
 * The first dimension of a and c are considered batch dimensions
 */
template <typename A, typename C, cpp_enable_if(all_single_precision<A>::value)>
void fft2_many(A&& a, C&& c) {
    static constexpr std::size_t N = decay_traits<A>::dimensions();

    std::size_t n1    = etl::dim<N - 2>(a);       //Size of the transform
    std::size_t n2    = etl::dim<N - 1>(a);       //Size of the transform
    std::size_t batch = etl::size(a) / (n1 * n2); //Number of batch

    auto a_complex = allocate<std::complex<float>>(etl::size(a));

    direct_copy(a.memory_start(), a.memory_end(), a_complex.get());

    mkl_detail::fft2_many_kernel(a_complex.get(), batch, n1, n2, c.memory_start());
}

/*!
 * \brief Perform many 2D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 *
 * The first dimension of a and c are considered batch dimensions
 */
template <typename A, typename C, cpp_enable_if(all_double_precision<A>::value)>
void fft2_many(A&& a, C&& c) {
    static constexpr std::size_t N = decay_traits<A>::dimensions();

    std::size_t n1    = etl::dim<N - 2>(a);       //Size of the transform
    std::size_t n2    = etl::dim<N - 1>(a);       //Size of the transform
    std::size_t batch = etl::size(a) / (n1 * n2); //Number of batch

    auto a_complex = allocate<std::complex<double>>(etl::size(a));

    direct_copy(a.memory_start(), a.memory_end(), a_complex.get());

    mkl_detail::fft2_many_kernel(a_complex.get(), batch, n1, n2, c.memory_start());
}

/*!
 * \brief Perform many 2D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 *
 * The first dimension of a and c are considered batch dimensions
 */
template <typename A, typename C, cpp_enable_if(all_complex<A>::value)>
void fft2_many(A&& a, C&& c) {
    static constexpr std::size_t N = decay_traits<A>::dimensions();

    std::size_t n1    = etl::dim<N - 2>(a);       //Size of the transform
    std::size_t n2    = etl::dim<N - 1>(a);       //Size of the transform
    std::size_t batch = etl::size(a) / (n1 * n2); //Number of batch

    mkl_detail::fft2_many_kernel(a.memory_start(), batch, n1, n2, c.memory_start());
}

/*!
 * \brief Perform many 2D Inverse FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 *
 * The first dimension of a and c are considered batch dimensions
 */
template <typename A, typename C>
void ifft2_many(A&& a, C&& c) {
    static constexpr std::size_t N = decay_traits<A>::dimensions();

    std::size_t n1    = etl::dim<N - 2>(a);       //Size of the transform
    std::size_t n2    = etl::dim<N - 1>(a);       //Size of the transform
    std::size_t batch = etl::size(a) / (n1 * n2); //Number of batch

    mkl_detail::ifft2_many_kernel(a.memory_start(), batch, n1, n2, c.memory_start());
}

/*!
 * \brief Perform the 2D Inverse FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C, cpp_enable_if(all_complex<A>::value)>
void ifft2(A&& a, C&& c) {
    mkl_detail::ifft2_kernel(a.memory_start(), etl::dim<0>(a), etl::dim<1>(a), c.memory_start());
}

/*!
 * \brief Perform the 2D Inverse FFT on a and store the real part of the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C>
void ifft2_real(A&& a, C&& c) {
    auto c_complex = allocate<std::complex<value_t<C>>>(etl::size(a));

    mkl_detail::ifft2_kernel(a.memory_start(), etl::dim<0>(a), etl::dim<1>(a), c_complex.get());

    for (std::size_t i = 0; i < etl::size(a); ++i) {
        c[i] = c_complex[i].real();
    }
}

/*!
 * \brief Perform the 2D full convolution of a with b and store the result in c
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename T>
void conv2_full(const opaque_memory<T, 2>& a, const opaque_memory<T, 2>& b, const opaque_memory<T, 2>& c) {
    mkl_detail::conv2_full_kernel(a.memory_start(), a.dim(0), a.dim(1), b.memory_start(), b.dim(0), b.dim(1), c.memory_start(), T(0.0));
}

/*!
 * \brief Perform the 2D full convolution of a with b and store the result in c,
 * with the flipped kernels of b.
 *
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename T>
void conv2_full_flipped(const opaque_memory<T, 2>& a, const opaque_memory<T, 2>& b, const opaque_memory<T, 2>& c) {
    etl::dyn_matrix<T, 2> prepared_b(b.dim(0), b.dim(1));

    std::copy(b.memory_start(), b.memory_end(), prepared_b.memory_start());

    prepared_b.fflip_inplace();

    mkl_detail::conv2_full_kernel(a.memory_start(), a.dim(0), a.dim(1), prepared_b.memory_start(), b.dim(0), b.dim(1), c.memory_start(), T(0.0));
}

/*!
 * \brief Perform the 2D full convolution of a with multiple kernels of b and store the result in c
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_full_multi(const opaque_memory<T, 2>& input, const opaque_memory<T, 3>& kernel, const opaque_memory<T, 3>& conv) {
    const auto K = kernel.dim(0);

    if (K) {
        const auto k_s = kernel.dim(1) * kernel.dim(2);
        const auto c_s = conv.dim(1) * conv.dim(2);

        const auto m1 = input.dim(0);
        const auto m2 = input.dim(1);

        const auto n1 = kernel.dim(1);
        const auto n2 = kernel.dim(2);

        const std::size_t s1   = m1 + n1 - 1;
        const std::size_t s2   = m2 + n2 - 1;
        const std::size_t size = s1 * s2;

        dyn_vector<etl::complex<T>> a_padded(size);

        for (std::size_t i = 0; i < m1; ++i) {
            direct_copy_n(input.memory_start() + i * m2, a_padded.memory_start() + i * s2, m2);
        }

        mkl_detail::inplace_fft2_kernel(reinterpret_cast<std::complex<T>*>(a_padded.memory_start()), s1, s2);

        auto batch_fun_k = [&](const size_t first, const size_t last) {
            SERIAL_SECTION {
                for (std::size_t k = first; k < last; ++k) {
                    const T* b = kernel.memory_start() + k * k_s;
                    T* c       = conv.memory_start() + k * c_s;

                    dyn_vector<etl::complex<T>> b_padded(size);

                    for (std::size_t i = 0; i < n1; ++i) {
                        direct_copy_n(b + i * n2, b_padded.memory_start() + i * s2, n2);
                    }

                    mkl_detail::inplace_fft2_kernel(reinterpret_cast<std::complex<T>*>(b_padded.memory_start()), s1, s2);

                    b_padded >>= a_padded;

                    mkl_detail::inplace_ifft2_kernel(reinterpret_cast<std::complex<T>*>(b_padded.memory_start()), s1, s2);

                    for (std::size_t i = 0; i < size; ++i) {
                        c[i] = b_padded[i].real;
                    }
                }
            }
        };

        if (etl::is_parallel) {
            dispatch_1d_any(select_parallel(K, 2), batch_fun_k, 0, K);
        } else {
            batch_fun_k(0, K);
        }
    }
}

/*!
 * \brief Perform the 2D full convolution of a with multiple kernels of b and store the result in c
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename T>
void conv2_full_multi_flipped(const opaque_memory<T, 2>& input, const opaque_memory<T, 3>& kernel, const opaque_memory<T, 3>& conv) {
    etl::dyn_matrix<T, 3> prepared_k(kernel.dim(0), kernel.dim(1), kernel.dim(2));

    std::copy(kernel.memory_start(), kernel.memory_end(), prepared_k.memory_start());

    prepared_k.deep_fflip_inplace();

    conv2_full_multi(input, prepared_k.direct(), conv);
}

/*!
 * \brief Perform the 4D full convolution of a with b and store the result in c
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename T>
void conv4_full(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv) {
    if (kernel.dim(1) > 0) {
        auto conv_i_inc = conv.dim(1) * conv.dim(2) * conv.dim(3);
        auto conv_c_inc = conv.dim(2) * conv.dim(3);

        auto kernel_k_inc = kernel.dim(1) * kernel.dim(2) * kernel.dim(3);
        auto kernel_c_inc = kernel.dim(2) * kernel.dim(3);

        auto input_i_inc = input.dim(1) * input.dim(2) * input.dim(3);
        auto input_k_inc = input.dim(2) * input.dim(3);

        const auto N = input.dim(0);
        const auto K = kernel.dim(0);
        const auto C = kernel.dim(1);

        const auto m1 = input.dim(2);
        const auto m2 = input.dim(3);

        const auto n1 = kernel.dim(2);
        const auto n2 = kernel.dim(3);

        const auto s1   = m1 + n1 - 1;
        const auto s2   = m2 + n2 - 1;
        const auto size = s1 * s2;

        std::fill(conv.memory_start(), conv.memory_end(), 0);

        dyn_matrix<etl::complex<T>, 3> b_padded(K, C, size);

        auto batch_fun_kc = [&](const size_t first, const size_t last) {
            for (std::size_t kc = first; kc < last; ++kc) {
                size_t k = kc / C;
                size_t c = kc % C;

                const T* b = kernel.memory_start() + k * kernel_k_inc + c * kernel_c_inc; // kernel(k)(c)

                b_padded(k)(c) = 0;
                for (std::size_t i = 0; i < n1; ++i) {
                    direct_copy_n(b + i * n2, b_padded(k)(c).memory_start() + i * s2, n2);
                }

                mkl_detail::inplace_fft2_kernel(reinterpret_cast<std::complex<T>*>(b_padded(k)(c).memory_start()), s1, s2);
            }
        };

        auto batch_fun_n = [&](const size_t first, const size_t last) {
            if (last - first) {
                SERIAL_SECTION {
                    for (std::size_t i = first; i < last; ++i) {
                        for (std::size_t k = 0; k < K; ++k) {
                            const T* a = input.memory_start() + i * input_i_inc + k * input_k_inc; // input(i)(k)

                            dyn_vector<etl::complex<T>> a_padded(size);
                            dyn_vector<etl::complex<T>> tmp(size);

                            a_padded = 0;

                            for (std::size_t i = 0; i < m1; ++i) {
                                direct_copy_n(a + i * m2, a_padded.memory_start() + i * s2, m2);
                            }

                            mkl_detail::inplace_fft2_kernel(reinterpret_cast<std::complex<T>*>(a_padded.memory_start()), s1, s2);

                            for (std::size_t c = 0; c < C; ++c) {
                                T* cc      = conv.memory_start() + i * conv_i_inc + c * conv_c_inc;       // conv(i)(c)

                                tmp = a_padded >> b_padded(k)(c);

                                mkl_detail::inplace_ifft2_kernel(reinterpret_cast<std::complex<T>*>(tmp.memory_start()), s1, s2);

                                for (std::size_t i = 0; i < size; ++i) {
                                    cc[i] += tmp[i].real;
                                }
                            }
                        }
                    }
                }
            }
        };

        if (etl::is_parallel) {
            dispatch_1d_any(select_parallel(K * C, 2), batch_fun_kc, 0, K * C);
            dispatch_1d_any(select_parallel(N, 2), batch_fun_n, 0, N);
        } else {
            batch_fun_kc(0, K * C);
            batch_fun_n(0, N);
        }
    }
}

/*!
 * \brief Perform the 4D full convolution of a with b and store the result in c
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename T>
void conv4_full_flipped(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv) {
    if (kernel.dim(1) > 0) {
        etl::dyn_matrix<T, 4> prepared_k(kernel.dim(0), kernel.dim(1), kernel.dim(2), kernel.dim(3));

        std::copy(kernel.memory_start(), kernel.memory_end(), prepared_k.memory_start());

        prepared_k.deep_fflip_inplace();

        conv4_full(input, prepared_k.direct(), conv);
    }
}

#else

//COVERAGE_EXCLUDE_BEGIN

/*!
 * \brief Perform the 1D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C>
void fft1(A&& a, C&& c) {
    cpp_unused(a);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 1D Inverse FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C>
void ifft1(A&& a, C&& c) {
    cpp_unused(a);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 1D Inverse FFT on a and store the real part of the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C>
void ifft1_real(A&& a, C&& c) {
    cpp_unused(a);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform many 1D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 *
 * The first dimension of a and c are considered batch dimensions
 */
template <typename A, typename C>
void fft1_many(A&& a, C&& c) {
    cpp_unused(a);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform many 1D Inverse FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 *
 * The first dimension of a and c are considered batch dimensions
 */
template <typename A, typename C>
void ifft1_many(A&& a, C&& c) {
    cpp_unused(a);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 2D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C>
void fft2(A&& a, C&& c) {
    cpp_unused(a);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 2D Inverse FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C>
void ifft2(A&& a, C&& c) {
    cpp_unused(a);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 2D Inverse FFT on a and store the real part of the result in c
 * \param a The input expression
 * \param c The output expression
 */
template <typename A, typename C>
void ifft2_real(A&& a, C&& c) {
    cpp_unused(a);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform many 2D FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 *
 * The first dimension of a and c are considered batch dimensions
 */
template <typename A, typename C>
void fft2_many(A&& a, C&& c) {
    cpp_unused(a);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform many 2D Inverse FFT on a and store the result in c
 * \param a The input expression
 * \param c The output expression
 *
 * The first dimension of a and c are considered batch dimensions
 */
template <typename A, typename C>
void ifft2_many(A&& a, C&& c) {
    cpp_unused(a);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 1D full convolution of a with b and store the result in c
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename A, typename B, typename C>
void conv1_full(A&& a, B&& b, C&& c) {
    cpp_unused(a);
    cpp_unused(b);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 2D full convolution of a with b and store the result in c
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename A, typename B, typename C>
void conv2_full(A&& a, B&& b, C&& c) {
    cpp_unused(a);
    cpp_unused(b);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 2D full convolution of a with multiple kernels of b and store the result in c
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename A, typename B, typename C>
void conv2_full_multi(A&& a, B&& b, C&& c) {
    cpp_unused(a);
    cpp_unused(b);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 2D full convolution of a with multiple kernels of b and store the result in c
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename A, typename B, typename C>
void conv2_full_multi_flipped(A&& a, B&& b, C&& c) {
    cpp_unused(a);
    cpp_unused(b);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 2D full convolution of a with b and store the result in c,
 * with the flipped kernels of b.
 *
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename A, typename B, typename C>
void conv2_full_flipped(A&& a, B&& b, C&& c) {
    cpp_unused(a);
    cpp_unused(b);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 4D full convolution of a with b and store the result in c
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename A, typename B, typename C>
void conv4_full(A&& a, B&& b, C&& c) {
    cpp_unused(a);
    cpp_unused(b);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

/*!
 * \brief Perform the 4D full convolution of a with the flipped kernels of b and store the result in c
 * \param a The input matrix
 * \param b The kernel matrix
 * \param c The output matrix
 */
template <typename A, typename B, typename C>
void conv4_full_flipped(A&& a, B&& b, C&& c) {
    cpp_unused(a);
    cpp_unused(b);
    cpp_unused(c);
    cpp_unreachable("Unsupported feature called: mkl fft");
}

//COVERAGE_EXCLUDE_END

#endif

} //end of namespace blas

} //end of namespace impl

} //end of namespace etl
