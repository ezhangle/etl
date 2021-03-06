//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

/*
 * SSE implementation of 1D and 2D convolutions
 *
 * Ideas:
 *  * the tmp_res vectors could be avoided by using hadd instructions
 *  * 1D convolution with no memory allocation could probably be worked out (needs to be benchmarked)
 *
 *  Notes:
 *  * FMA for the 1D convolution is making is slower for some reason
 */

#if defined(ETL_VECTORIZE_IMPL) && defined(__SSE3__)

#include "common.hpp"
#include "etl/impl/common/conv.hpp"

#endif

namespace etl {

namespace impl {

namespace sse {

#if defined(ETL_VECTORIZE_IMPL) && defined(__SSE3__)

#ifdef __clang__
#define _mm_undefined_ps _mm_setzero_ps
#endif

inline void conv2_valid_flipped_border(const double* in, std::size_t n1, std::size_t n2, const double* kernel, std::size_t m1, std::size_t m2, double* out, double beta, std::size_t i, std::size_t j, size_t s1, size_t s2, size_t p1, size_t p2) {
    const std::size_t c2 = (n2 - m2 + 2 * p2) / s2 + 1;

    double temp = 0.0;

    const auto s_i = i * s1;
    const auto s_j = j * s2;

    for (std::size_t k = 0; k < m1; ++k) {
        for (std::size_t l = 0; l < m2; ++l) {
            if(s_i + k >= p1 && (s_i + k) - p1 < n1 && s_j + l >= p2 && (s_j + l) - p2 < n2){
                const size_t i_i = (s_i + k) - p1;
                const size_t i_j = (s_j + l) - p2;

                temp += in[i_i * n2 + i_j] * kernel[k * m2 + l];
            }
        }
    }

    if(beta == 0.0){
        out[i * c2 + j] = temp;
    } else {
        out[i * c2 + j] = beta * out[i * c2 + j] + temp;
    }
}

inline void conv2_valid_flipped_micro_kernel(const double* in, std::size_t n1, std::size_t n2, const double* kernel, std::size_t m1, std::size_t m2, double* out, double beta, size_t s1, size_t s2, size_t p1, size_t p2) {
    const std::size_t c1 = (n1 - m1 + 2 * p1) / s1 + 1;
    const std::size_t c2 = (n2 - m2 + 2 * p2) / s2 + 1;

    if(p1 || p2){
        for (std::size_t i = 0; i < p1; ++i) {
            for (std::size_t j = 0; j < c2; ++j) {
                conv2_valid_flipped_border(in, n1, n2, kernel, m1, m2, out, beta, i, j, s1, s2, p1, p2);
            }
        }

        for (std::size_t i = c1 - p1; i < c1; ++i) {
            for (std::size_t j = 0; j < c2; ++j) {
                conv2_valid_flipped_border(in, n1, n2, kernel, m1, m2, out, beta, i, j, s1, s2, p1, p2);
            }
        }

        for (std::size_t j = 0; j < p2; ++j) {
            for (std::size_t i = p1; i < c1 - p1; ++i) {
                conv2_valid_flipped_border(in, n1, n2, kernel, m1, m2, out, beta, i, j, s1, s2, p1, p2);
            }
        }

        for (std::size_t j = c2 - p2; j < c2; ++j) {
            for (std::size_t i = p1; i < c1 - p1; ++i) {
                conv2_valid_flipped_border(in, n1, n2, kernel, m1, m2, out, beta, i, j, s1, s2, p1, p2);
            }
        }
    }

    if(beta == 0.0){
        for (std::size_t i = p1; i < c1 - p1; ++i) {
            for (std::size_t j = p2; j + 3 < c2 - p2; j += 4) {
                __m128d r1 = _mm_setzero_pd();
                __m128d r2 = _mm_setzero_pd();
                __m128d r3 = _mm_setzero_pd();
                __m128d r4 = _mm_setzero_pd();

                const auto i_i = i * s1 - p1;
                const auto i_j = j * s2 - p2;

                for (std::size_t k = 0; k < m1; ++k) {
                    for (std::size_t l = 0; l + 1 < m2; l += 2) {
                        __m128d k1 = _mm_loadu_pd(kernel + k * m2 + l);

                        __m128d i1 = _mm_loadu_pd(in + (i_i + k) * n2 + i_j + 0 + l);
                        __m128d i2 = _mm_loadu_pd(in + (i_i + k) * n2 + i_j + 1 + l);
                        __m128d i3 = _mm_loadu_pd(in + (i_i + k) * n2 + i_j + 2 + l);
                        __m128d i4 = _mm_loadu_pd(in + (i_i + k) * n2 + i_j + 3 + l);

                        __m128d t1 = _mm_mul_pd(k1, i1);
                        __m128d t2 = _mm_mul_pd(k1, i2);
                        __m128d t3 = _mm_mul_pd(k1, i3);
                        __m128d t4 = _mm_mul_pd(k1, i4);

                        r1  = _mm_add_pd(r1, t1);
                        r2  = _mm_add_pd(r2, t2);
                        r3  = _mm_add_pd(r3, t3);
                        r4  = _mm_add_pd(r4, t4);
                    }
                }

                out[i * c2 + j + 0] = detail::mm_hadd_sd(r1);
                out[i * c2 + j + 1] = detail::mm_hadd_sd(r2);
                out[i * c2 + j + 2] = detail::mm_hadd_sd(r3);
                out[i * c2 + j + 3] = detail::mm_hadd_sd(r4);
            }

            for (std::size_t j = (c2 - p2) - (c2 - 2 * p2) % 4; j < c2 - p2; ++j) {
                __m128d r1 = _mm_setzero_pd();

                const auto i_i = i * s1 - p1;
                const auto i_j = j * s2 - p2;

                for (std::size_t k = 0; k < m1; ++k) {
                    for (std::size_t l = 0; l + 1 < m2; l += 2) {
                        __m128d tmp1 = _mm_loadu_pd(in + (i_i + k) * n2 + i_j + l);
                        __m128d tmp2 = _mm_loadu_pd(kernel + k * m2 + l);
                        __m128d tmp4 = _mm_mul_pd(tmp2, tmp1);
                        r1  = _mm_add_pd(r1, tmp4);
                    }
                }

                out[i * c2 + j] = detail::mm_hadd_sd(r1);
            }
        }
    } else {
        for (std::size_t i = p1; i < c1 - p1; ++i) {
            for (std::size_t j = p2; j + 3 < c2 - p2; j += 4) {
                __m128d r1 = _mm_setzero_pd();
                __m128d r2 = _mm_setzero_pd();
                __m128d r3 = _mm_setzero_pd();
                __m128d r4 = _mm_setzero_pd();

                const auto i_i = i * s1 - p1;
                const auto i_j = j * s2 - p2;

                for (std::size_t k = 0; k < m1; ++k) {
                    for (std::size_t l = 0; l + 1 < m2; l += 2) {
                        __m128d k1 = _mm_loadu_pd(kernel + k * m2 + l);

                        __m128d i1 = _mm_loadu_pd(in + (i_i + k) * n2 + i_j + 0 + l);
                        __m128d i2 = _mm_loadu_pd(in + (i_i + k) * n2 + i_j + 1 + l);
                        __m128d i3 = _mm_loadu_pd(in + (i_i + k) * n2 + i_j + 2 + l);
                        __m128d i4 = _mm_loadu_pd(in + (i_i + k) * n2 + i_j + 3 + l);

                        __m128d t1 = _mm_mul_pd(k1, i1);
                        __m128d t2 = _mm_mul_pd(k1, i2);
                        __m128d t3 = _mm_mul_pd(k1, i3);
                        __m128d t4 = _mm_mul_pd(k1, i4);

                        r1  = _mm_add_pd(r1, t1);
                        r2  = _mm_add_pd(r2, t2);
                        r3  = _mm_add_pd(r3, t3);
                        r4  = _mm_add_pd(r4, t4);
                    }
                }

                out[i * c2 + j + 0] = beta * out[i * c2 + j + 0] + detail::mm_hadd_sd(r1);
                out[i * c2 + j + 1] = beta * out[i * c2 + j + 1] + detail::mm_hadd_sd(r2);
                out[i * c2 + j + 2] = beta * out[i * c2 + j + 2] + detail::mm_hadd_sd(r3);
                out[i * c2 + j + 3] = beta * out[i * c2 + j + 3] + detail::mm_hadd_sd(r4);
            }

            for (std::size_t j = (c2 - p2) - (c2 - 2 * p2) % 4; j < c2 - p2; ++j) {
                __m128d r1 = _mm_setzero_pd();

                const auto i_i = i * s1 - p1;
                const auto i_j = j * s2 - p2;

                for (std::size_t k = 0; k < m1; ++k) {
                    for (std::size_t l = 0; l + 1 < m2; l += 2) {
                        __m128d tmp1 = _mm_loadu_pd(in + (i_i + k) * n2 + i_j + l);
                        __m128d tmp2 = _mm_loadu_pd(kernel + k * m2 + l);
                        __m128d tmp4 = _mm_mul_pd(tmp2, tmp1);
                        r1  = _mm_add_pd(r1, tmp4);
                    }
                }

                out[i * c2 + j] = beta * out[i * c2 + j] + detail::mm_hadd_sd(r1);
            }
        }
    }

    if (!padding_impl && m2 % 2 != 0) {
        for (std::size_t i = p1; i < c1 - p1; ++i) {
            for (std::size_t j = p2; j < c2 - p2; ++j) {
                double temp = 0.0;

                const auto i_i = i * s1 - p1;
                const auto i_j = j * s2 - p2;

                for (std::size_t k = 0; k < m1; ++k) {
                    const auto l = m2 - 1;
                    temp += in[(i_i + k) * n2 + i_j + l] * kernel[k * m2 + l];
                }

                out[i * c2 + j] += temp;
            }
        }
    }
}

inline void conv2_valid_micro_kernel(const double* in, std::size_t n1, std::size_t n2, const double* kernel, std::size_t m1, std::size_t m2, double* out, double beta, size_t s1, size_t s2, size_t p1, size_t p2) {
    auto kernel_reverse = aligned_allocate_auto<double>(m1 * m2);

    std::reverse_copy(kernel, kernel + m1 * m2, kernel_reverse.get());

    conv2_valid_flipped_micro_kernel(in, n1, n2, kernel_reverse.get(), m1, m2, out, beta, s1, s2, p1, p2);
}

inline void conv2_same_micro_kernel(const double* in, std::size_t n1, std::size_t n2, const double* kernel, std::size_t m1, std::size_t m2, double* out) {
    std::size_t c1 = n1;
    std::size_t c2 = n2;

    for (std::size_t i = 0; i < c1; ++i) {
        std::size_t k_lo = std::max<int>(0, i - (m1 - 1) / 2);
        std::size_t k_hi = std::min<int>(n1 - 1, i + m1 / 2) + 1;

        for (std::size_t j = 0; j < c2; ++j) {
            std::size_t l_lo = std::max<int>(0, j - (m2 - 1) / 2);
            std::size_t l_hi = std::min<int>(n2 - 1, j + m2 / 2) + 1;

            __m128d r1 = _mm_setzero_pd();

            for (std::size_t k = k_lo; k < k_hi; ++k) {
                for (std::size_t l = l_lo; l + 1 < l_hi; l += 2) {
                    __m128d i1 = _mm_loadu_pd(in + k * n2 + l);

                    __m128d t2 = _mm_loadu_pd(kernel + (i - k + m1 / 2) * m2 + (j - (l + 1) + m2 / 2));
                    __m128d k1 = _mm_shuffle_pd(t2, t2, _MM_SHUFFLE2(0, 1));

                    __m128d t1 = _mm_mul_pd(k1, i1);
                    r1  = _mm_add_pd(r1, t1);
                }
            }

            out[i * c2 + j] = detail::mm_hadd_sd(r1);

            double temp = 0.0;

            if ((l_hi - l_lo) % 2 != 0) {
                auto rem = (l_hi - l_lo) % 2;
                auto l = l_hi - rem;
                for (std::size_t k = k_lo; k < k_hi; ++k) {
                    temp += in[k * n2 + l] * kernel[(i - k + m1 / 2) * m2 + (j - l + m2 / 2)];
                }
            }

            out[i * c2 + j] += temp;
        }
    }
}

inline void conv2_same_flipped_micro_kernel(const double* in, std::size_t n1, std::size_t n2, const double* kernel, std::size_t m1, std::size_t m2, double* out) {
    auto kernel_reverse = aligned_allocate_auto<double>(m1 * m2);

    std::reverse_copy(kernel, kernel + m1 * m2, kernel_reverse.get());

    conv2_same_micro_kernel(in, n1, n2, kernel_reverse.get(), m1, m2, out);
}

inline void conv2_full_micro_kernel(const double* in, std::size_t n1, std::size_t n2, const double* kernel, std::size_t m1, std::size_t m2, double* out, double beta) {
    std::size_t c1 = n1 + m1 - 1;
    std::size_t c2 = n2 + m2 - 1;

    if(beta == 0.0){
        for (std::size_t i = 0; i < c1; ++i) {
            auto k_lo = std::max<int>(0, i - m1 + 1);
            auto k_hi = std::min(n1 - 1, i) + 1;

            for (std::size_t j = 0; j < c2; ++j) {
                auto l_lo = std::max<int>(0, j - m2 + 1);
                auto l_hi = std::min(n2 - 1, j) + 1;

                __m128d r1 = _mm_setzero_pd();

                for (std::size_t k = k_lo; k < k_hi; ++k) {
                    for (std::size_t l = l_lo; l + 1 < l_hi; l += 2) {
                        __m128d i1 = _mm_loadu_pd(in + k * n2 + l);
                        __m128d t2 = _mm_loadu_pd(kernel + (i - k) * m2 + (j - (l + 1)));
                        __m128d k1 = _mm_shuffle_pd(t2, t2, _MM_SHUFFLE2(0, 1));
                        __m128d t1 = _mm_mul_pd(k1, i1);
                        r1  = _mm_add_pd(r1, t1);
                    }
                }

                out[i * c2 + j] = detail::mm_hadd_sd(r1);

                double temp = 0.0;

                if ((l_hi - l_lo) % 2 != 0) {
                    for (std::size_t k = k_lo; k < k_hi; ++k) {
                        temp += in[k * n2 + l_hi - 1] * kernel[(i - k) * m2 + (j - (l_hi - 1))];
                    }
                }

                out[i * c2 + j] += temp;
            }
        }
    } else {
        for (std::size_t i = 0; i < c1; ++i) {
            auto k_lo = std::max<int>(0, i - m1 + 1);
            auto k_hi = std::min(n1 - 1, i) + 1;

            for (std::size_t j = 0; j < c2; ++j) {
                auto l_lo = std::max<int>(0, j - m2 + 1);
                auto l_hi = std::min(n2 - 1, j) + 1;

                __m128d r1 = _mm_setzero_pd();

                for (std::size_t k = k_lo; k < k_hi; ++k) {
                    for (std::size_t l = l_lo; l + 1 < l_hi; l += 2) {
                        __m128d i1 = _mm_loadu_pd(in + k * n2 + l);
                        __m128d t2 = _mm_loadu_pd(kernel + (i - k) * m2 + (j - (l + 1)));
                        __m128d k1 = _mm_shuffle_pd(t2, t2, _MM_SHUFFLE2(0, 1));
                        __m128d t1 = _mm_mul_pd(k1, i1);
                        r1  = _mm_add_pd(r1, t1);
                    }
                }

                out[i * c2 + j] = beta * out[i * c2 + j] + detail::mm_hadd_sd(r1);

                double temp = 0.0;

                if ((l_hi - l_lo) % 2 != 0) {
                    for (std::size_t k = k_lo; k < k_hi; ++k) {
                        temp += in[k * n2 + l_hi - 1] * kernel[(i - k) * m2 + (j - (l_hi - 1))];
                    }
                }

                out[i * c2 + j] += temp;
            }
        }
    }
}

inline void conv2_full_flipped_micro_kernel(const double* in, std::size_t n1, std::size_t n2, const double* kernel, std::size_t m1, std::size_t m2, double* out, double beta) {
    std::size_t c1 = n1 + m1 - 1;
    std::size_t c2 = n2 + m2 - 1;

    if (beta == 0.0) {
        for (std::size_t i = 0; i < c1; ++i) {
            auto k_lo = std::max<int>(0, i - m1 + 1);
            auto k_hi = std::min(n1 - 1, i) + 1;

            for (std::size_t j = 0; j < c2; ++j) {
                auto l_lo = std::max<int>(0, j - m2 + 1);
                auto l_hi = std::min(n2 - 1, j) + 1;

                __m128d r1 = _mm_setzero_pd();

                for (std::size_t k = k_lo; k < k_hi; ++k) {
                    for (std::size_t l = l_lo; l + 1 < l_hi; l += 2) {
                        __m128d i1 = _mm_loadu_pd(in + k * n2 + l);
                        __m128d k1 = _mm_loadu_pd(kernel + (m1 - 1 - (i - k)) * m2 + (m2 - 1 - (j - l)));
                        __m128d t1 = _mm_mul_pd(k1, i1);
                        r1         = _mm_add_pd(r1, t1);
                    }
                }

                out[i * c2 + j] = detail::mm_hadd_sd(r1);

                double temp = 0.0;

                if ((l_hi - l_lo) % 2 != 0) {
                    for (std::size_t k = k_lo; k < k_hi; ++k) {
                        temp += in[k * n2 + l_hi - 1] * kernel[(m1 - 1 - (i - k)) * m2 + (m2 - 1 - (j - (l_hi - 1)))];
                    }
                }

                out[i * c2 + j] += temp;
            }
        }
    } else {
        for (std::size_t i = 0; i < c1; ++i) {
            auto k_lo = std::max<int>(0, i - m1 + 1);
            auto k_hi = std::min(n1 - 1, i) + 1;

            for (std::size_t j = 0; j < c2; ++j) {
                auto l_lo = std::max<int>(0, j - m2 + 1);
                auto l_hi = std::min(n2 - 1, j) + 1;

                __m128d r1 = _mm_setzero_pd();

                for (std::size_t k = k_lo; k < k_hi; ++k) {
                    for (std::size_t l = l_lo; l + 1 < l_hi; l += 2) {
                        __m128d i1 = _mm_loadu_pd(in + k * n2 + l);
                        __m128d k1 = _mm_loadu_pd(kernel + (m1 - 1 - (i - k)) * m2 + (m2 - 1 - (j - l)));
                        __m128d t1 = _mm_mul_pd(k1, i1);
                        r1         = _mm_add_pd(r1, t1);
                    }
                }

                out[i * c2 + j] = beta * out[i * c2 + j] + detail::mm_hadd_sd(r1);

                double temp = 0.0;

                if ((l_hi - l_lo) % 2 != 0) {
                    for (std::size_t k = k_lo; k < k_hi; ++k) {
                        temp += in[k * n2 + l_hi - 1] * kernel[(m1 - 1 - (i - k)) * m2 + (m2 - 1 - (j - (l_hi - 1)))];
                    }
                }

                out[i * c2 + j] += temp;
            }
        }
    }
}

inline void conv2_valid_flipped_border(const float* in, std::size_t n1, std::size_t n2, const float* kernel, std::size_t m1, std::size_t m2, float* out, float beta, std::size_t i, std::size_t j, size_t s1, size_t s2, size_t p1, size_t p2) {
    const std::size_t c2 = (n2 - m2 + 2 * p2) / s2 + 1;

    float temp = 0.0f;

    const auto s_i = i * s1;
    const auto s_j = j * s2;

    for (std::size_t k = 0; k < m1; ++k) {
        for (std::size_t l = 0; l < m2; ++l) {
            if(s_i + k >= p1 && (s_i + k) - p1 < n1 && s_j + l >= p2 && (s_j + l) - p2 < n2){
                const size_t i_i = (s_i + k) - p1;
                const size_t i_j = (s_j + l) - p2;

                temp += in[i_i * n2 + i_j] * kernel[k * m2 + l];
            }
        }
    }

    if(beta == 0.0f){
        out[i * c2 + j] = temp;
    } else {
        out[i * c2 + j] = beta * out[i * c2 + j] + temp;
    }
}

inline void conv2_valid_flipped_micro_kernel(const float* in, std::size_t n1, std::size_t n2, const float* kernel, std::size_t m1, std::size_t m2, float* out, float beta, size_t s1, size_t s2, size_t p1, size_t p2) {
    const std::size_t c1 = (n1 - m1 + 2 * p1) / s1 + 1;
    const std::size_t c2 = (n2 - m2 + 2 * p2) / s2 + 1;

    if(p1 || p2){
        for (std::size_t i = 0; i < p1; ++i) {
            for (std::size_t j = 0; j < c2; ++j) {
                conv2_valid_flipped_border(in, n1, n2, kernel, m1, m2, out, beta, i, j, s1, s2, p1, p2);
            }
        }

        for (std::size_t i = c1 - p1; i < c1; ++i) {
            for (std::size_t j = 0; j < c2; ++j) {
                conv2_valid_flipped_border(in, n1, n2, kernel, m1, m2, out, beta, i, j, s1, s2, p1, p2);
            }
        }

        for (std::size_t j = 0; j < p2; ++j) {
            for (std::size_t i = p1; i < c1 - p1; ++i) {
                conv2_valid_flipped_border(in, n1, n2, kernel, m1, m2, out, beta, i, j, s1, s2, p1, p2);
            }
        }

        for (std::size_t j = c2 - p2; j < c2; ++j) {
            for (std::size_t i = p1; i < c1 - p1; ++i) {
                conv2_valid_flipped_border(in, n1, n2, kernel, m1, m2, out, beta, i, j, s1, s2, p1, p2);
            }
        }
    }

    if(beta == 0.0f){
        for (std::size_t i = p1; i < c1 - p1; ++i) {
            for (std::size_t j = p2; j + 3 < c2 - p2; j += 4) {
                __m128 r1 = _mm_setzero_ps();
                __m128 r2 = _mm_setzero_ps();
                __m128 r3 = _mm_setzero_ps();
                __m128 r4 = _mm_setzero_ps();

                const auto i_i = i * s1 - p1;
                const auto i_j = j * s2 - p2;

                for (std::size_t k = 0; k < m1; ++k) {
                    for (std::size_t l = 0; l + 3 < m2; l += 4) {
                        __m128 k1 = _mm_loadu_ps(kernel + k * m2 + l);

                        __m128 i1 = _mm_loadu_ps(in + (k + i_i) * n2 + l + i_j + 0);
                        __m128 i2 = _mm_loadu_ps(in + (k + i_i) * n2 + l + i_j + 1);
                        __m128 i3 = _mm_loadu_ps(in + (k + i_i) * n2 + l + i_j + 2);
                        __m128 i4 = _mm_loadu_ps(in + (k + i_i) * n2 + l + i_j + 3);

                        __m128 t1 = _mm_mul_ps(k1, i1);
                        __m128 t2 = _mm_mul_ps(k1, i2);
                        __m128 t3 = _mm_mul_ps(k1, i3);
                        __m128 t4 = _mm_mul_ps(k1, i4);

                        r1  = _mm_add_ps(r1, t1);
                        r2  = _mm_add_ps(r2, t2);
                        r3  = _mm_add_ps(r3, t3);
                        r4  = _mm_add_ps(r4, t4);
                    }
                }

                out[i * c2 + j + 0] = detail::mm_hadd_ss(r1);
                out[i * c2 + j + 1] = detail::mm_hadd_ss(r2);
                out[i * c2 + j + 2] = detail::mm_hadd_ss(r3);
                out[i * c2 + j + 3] = detail::mm_hadd_ss(r4);
            }

            for (std::size_t j = (c2 - p2) - (c2 - 2 * p2) % 4; j < c2 - p2; ++j) {
                __m128 r1 = _mm_setzero_ps();

                const auto i_i = i * s1 - p1;
                const auto i_j = j * s2 - p2;

                for (std::size_t k = 0; k < m1; ++k) {
                    for (std::size_t l = 0; l + 3 < m2; l += 4) {
                        __m128 k1 = _mm_loadu_ps(kernel + k * m2 + l);

                        __m128 i1 = _mm_loadu_ps(in + (k + i_i) * n2 + l + i_j);

                        __m128 t1 = _mm_mul_ps(k1, i1);

                        r1  = _mm_add_ps(r1, t1);
                    }
                }

                out[i * c2 + j] = detail::mm_hadd_ss(r1);
            }
        }
    } else {
        for (std::size_t i = p1; i < c1 - p1; ++i) {
            for (std::size_t j = p2; j + 3 < c2 - p2; j += 4) {
                __m128 r1 = _mm_setzero_ps();
                __m128 r2 = _mm_setzero_ps();
                __m128 r3 = _mm_setzero_ps();
                __m128 r4 = _mm_setzero_ps();

                const auto i_i = i * s1 - p1;
                const auto i_j = j * s2 - p2;

                for (std::size_t k = 0; k < m1; ++k) {
                    for (std::size_t l = 0; l + 3 < m2; l += 4) {
                        __m128 k1 = _mm_loadu_ps(kernel + k * m2 + l);

                        __m128 i1 = _mm_loadu_ps(in + (k + i_i) * n2 + l + i_j + 0);
                        __m128 i2 = _mm_loadu_ps(in + (k + i_i) * n2 + l + i_j + 1);
                        __m128 i3 = _mm_loadu_ps(in + (k + i_i) * n2 + l + i_j + 2);
                        __m128 i4 = _mm_loadu_ps(in + (k + i_i) * n2 + l + i_j + 3);

                        __m128 t1 = _mm_mul_ps(k1, i1);
                        __m128 t2 = _mm_mul_ps(k1, i2);
                        __m128 t3 = _mm_mul_ps(k1, i3);
                        __m128 t4 = _mm_mul_ps(k1, i4);

                        r1  = _mm_add_ps(r1, t1);
                        r2  = _mm_add_ps(r2, t2);
                        r3  = _mm_add_ps(r3, t3);
                        r4  = _mm_add_ps(r4, t4);
                    }
                }

                out[i * c2 + j + 0] = beta * out[i * c2 + j + 0] + detail::mm_hadd_ss(r1);
                out[i * c2 + j + 1] = beta * out[i * c2 + j + 1] + detail::mm_hadd_ss(r2);
                out[i * c2 + j + 2] = beta * out[i * c2 + j + 2] + detail::mm_hadd_ss(r3);
                out[i * c2 + j + 3] = beta * out[i * c2 + j + 3] + detail::mm_hadd_ss(r4);
            }

            for (std::size_t j = (c2 - p2) - (c2 - 2 * p2) % 4; j < c2 - p2; ++j) {
                __m128 r1 = _mm_setzero_ps();

                const auto i_i = i * s1 - p1;
                const auto i_j = j * s2 - p2;

                for (std::size_t k = 0; k < m1; ++k) {
                    for (std::size_t l = 0; l + 3 < m2; l += 4) {
                        __m128 k1 = _mm_loadu_ps(kernel + k * m2 + l);

                        __m128 i1 = _mm_loadu_ps(in + (k + i_i) * n2 + l + i_j);

                        __m128 t1 = _mm_mul_ps(k1, i1);

                        r1  = _mm_add_ps(r1, t1);
                    }
                }

                out[i * c2 + j] = beta * out[i * c2 + j] + detail::mm_hadd_ss(r1);
            }
        }
    }

    if (!padding_impl && m2 % 4 != 0) {
        for (std::size_t i = p1; i < c1 - p1; ++i) {
            for (std::size_t j = p2; j < c2 - p2; ++j) {
                float temp = 0.0;

                const auto i_i = i * s1 - p1;
                const auto i_j = j * s2 - p2;

                for (std::size_t k = 0; k < m1; ++k) {
                    for (std::size_t l = m2 - m2 % 4; l < m2; ++l) {
                        temp += in[(k + i_i) * n2 + l + i_j] * kernel[k * m2 + l];
                    }
                }

                out[i * c2 + j] += temp;
            }
        }
    }
}

inline void conv2_valid_micro_kernel(const float* in, std::size_t n1, std::size_t n2, const float* kernel, std::size_t m1, std::size_t m2, float* out, float beta, size_t s1, size_t s2, size_t p1, size_t p2) {
    auto kernel_reverse = aligned_allocate_auto<float>(m1 * m2);

    std::reverse_copy(kernel, kernel + m1 * m2, kernel_reverse.get());

    conv2_valid_flipped_micro_kernel(in, n1, n2, kernel_reverse.get(), m1, m2, out, beta, s1, s2, p1, p2);
}

inline void conv2_same_micro_kernel(const float* in, std::size_t n1, std::size_t n2, const float* kernel, std::size_t m1, std::size_t m2, float* out) {
    std::size_t c1 = n1;
    std::size_t c2 = n2;

    for (std::size_t i = 0; i < c1; ++i) {
        auto k_lo = std::max<int>(0, i - (m1 - 1) / 2);
        auto k_hi = std::min<int>(n1 - 1, i + m1 / 2) + 1;

        for (std::size_t j = 0; j < c2; ++j) {
            auto l_lo = std::max<int>(0, j - (m2 - 1) / 2);
            auto l_hi = std::min<int>(n2 - 1, j + m2 / 2) + 1;

            __m128 r1 = _mm_setzero_ps();

            for (int k = k_lo; k < k_hi; ++k) {
                for (std::size_t l = l_lo; l + 3 < static_cast<std::size_t>(l_hi); l += 4) {
                    __m128 i1 = _mm_loadu_ps(in + k * n2 + l);
                    __m128 t2 = _mm_loadu_ps(kernel + (i - k + m1 / 2) * m2 + (j - (l + 3) + m2 / 2));
                    __m128 k1 = _mm_shuffle_ps(t2, t2, _MM_SHUFFLE(0, 1, 2, 3));

                    __m128 t1 = _mm_mul_ps(k1, i1);
                    r1  = _mm_add_ps(r1, t1);
                }
            }

            out[i * c2 + j] = detail::mm_hadd_ss(r1);

            float temp = 0.0;

            if ((l_hi - l_lo) % 4 != 0) {
                auto rem = (l_hi - l_lo) % 4;
                for (int k = k_lo; k < k_hi; ++k) {
                    for (std::size_t l = l_hi - rem; l < static_cast<std::size_t>(l_hi); ++l) {
                        temp += in[k * n2 + l] * kernel[(i - k + m1 / 2) * m2 + (j - l + m2 / 2)];
                    }
                }
            }

            out[i * c2 + j] += temp;
        }
    }
}

inline void conv2_same_flipped_micro_kernel(const float* in, std::size_t n1, std::size_t n2, const float* kernel, std::size_t m1, std::size_t m2, float* out) {
    auto kernel_reverse = aligned_allocate_auto<float>(m1 * m2);

    std::reverse_copy(kernel, kernel + m1 * m2, kernel_reverse.get());

    conv2_same_micro_kernel(in, n1, n2, kernel_reverse.get(), m1, m2, out);
}

inline void conv2_full_micro_kernel(const float* in, std::size_t n1, std::size_t n2, const float* kernel, std::size_t m1, std::size_t m2, float* out, float beta) {
    std::size_t c1 = n1 + m1 - 1;
    std::size_t c2 = n2 + m2 - 1;

    if(beta == 0.0){
        for (std::size_t i = 0; i < c1; ++i) {
            auto k_lo = std::max<int>(0, i - m1 + 1);
            auto k_hi = std::min(n1 - 1, i) + 1;

            for (std::size_t j = 0; j < c2; ++j) {
                auto l_lo = std::max<int>(0, j - m2 + 1);
                auto l_hi = std::min(n2 - 1, j) + 1;

                __m128 r1 = _mm_setzero_ps();

                for (std::size_t k = k_lo; k < k_hi; ++k) {
                    for (std::size_t l = l_lo; l + 3 < l_hi; l += 4) {
                        __m128 i1 = _mm_loadu_ps(in + k * n2 + l);
                        __m128 t2 = _mm_loadu_ps(kernel + (i - k) * m2 + (j - (l + 3)));
                        __m128 k1 = _mm_shuffle_ps(t2, t2, _MM_SHUFFLE(0, 1, 2, 3));
                        __m128 t1 = _mm_mul_ps(k1, i1);
                        r1        = _mm_add_ps(r1, t1);
                    }
                }

                out[i * c2 + j] = detail::mm_hadd_ss(r1);

                double temp = 0.0;

                if ((l_hi - l_lo) % 4 != 0) {
                    auto rem = (l_hi - l_lo) % 4;
                    for (std::size_t k = k_lo; k < k_hi; ++k) {
                        for (std::size_t l = l_hi - rem; l < l_hi; ++l) {
                            temp += in[k * n2 + l] * kernel[(i - k) * m2 + (j - l)];
                        }
                    }
                }

                out[i * c2 + j] += temp;
            }
        }
    } else {
        for (std::size_t i = 0; i < c1; ++i) {
            auto k_lo = std::max<int>(0, i - m1 + 1);
            auto k_hi = std::min(n1 - 1, i) + 1;

            for (std::size_t j = 0; j < c2; ++j) {
                auto l_lo = std::max<int>(0, j - m2 + 1);
                auto l_hi = std::min(n2 - 1, j) + 1;

                __m128 r1 = _mm_setzero_ps();

                for (std::size_t k = k_lo; k < k_hi; ++k) {
                    for (std::size_t l = l_lo; l + 3 < l_hi; l += 4) {
                        __m128 i1 = _mm_loadu_ps(in + k * n2 + l);
                        __m128 t2 = _mm_loadu_ps(kernel + (i - k) * m2 + (j - (l + 3)));
                        __m128 k1 = _mm_shuffle_ps(t2, t2, _MM_SHUFFLE(0, 1, 2, 3));
                        __m128 t1 = _mm_mul_ps(k1, i1);
                        r1        = _mm_add_ps(r1, t1);
                    }
                }

                out[i * c2 + j] = beta * out[i * c2 + j] + detail::mm_hadd_ss(r1);

                double temp = 0.0;

                if ((l_hi - l_lo) % 4 != 0) {
                    auto rem = (l_hi - l_lo) % 4;
                    for (std::size_t k = k_lo; k < k_hi; ++k) {
                        for (std::size_t l = l_hi - rem; l < l_hi; ++l) {
                            temp += in[k * n2 + l] * kernel[(i - k) * m2 + (j - l)];
                        }
                    }
                }

                out[i * c2 + j] += temp;
            }
        }
    }
}

inline void conv2_full_flipped_micro_kernel(const float* in, std::size_t n1, std::size_t n2, const float* kernel, std::size_t m1, std::size_t m2, float* out, float beta) {
    std::size_t c1 = n1 + m1 - 1;
    std::size_t c2 = n2 + m2 - 1;

    if(beta == 0.0){
        for (std::size_t i = 0; i < c1; ++i) {
            auto k_lo = std::max<int>(0, i - m1 + 1);
            auto k_hi = std::min(n1 - 1, i) + 1;

            for (std::size_t j = 0; j < c2; ++j) {
                auto l_lo = std::max<int>(0, j - m2 + 1);
                auto l_hi = std::min(n2 - 1, j) + 1;

                __m128 r1 = _mm_setzero_ps();

                for (std::size_t k = k_lo; k < k_hi; ++k) {
                    for (std::size_t l = l_lo; l + 3 < l_hi; l += 4) {
                        __m128 i1 = _mm_loadu_ps(in + k * n2 + l);
                        __m128 k1 = _mm_loadu_ps(kernel + (m1 - 1 - (i - k)) * m2 + (m2 - 1 - (j - l)));
                        __m128 t1 = _mm_mul_ps(k1, i1);
                        r1        = _mm_add_ps(r1, t1);
                    }
                }

                out[i * c2 + j] = detail::mm_hadd_ss(r1);

                double temp = 0.0;

                if ((l_hi - l_lo) % 4 != 0) {
                    auto rem = (l_hi - l_lo) % 4;
                    for (std::size_t k = k_lo; k < k_hi; ++k) {
                        for (std::size_t l = l_hi - rem; l < l_hi; ++l) {
                            temp += in[k * n2 + l] * kernel[(m1 - 1 - (i - k)) * m2 + (m2 - 1 - (j - l))];
                        }
                    }
                }

                out[i * c2 + j] += temp;
            }
        }
    } else {
        for (std::size_t i = 0; i < c1; ++i) {
            auto k_lo = std::max<int>(0, i - m1 + 1);
            auto k_hi = std::min(n1 - 1, i) + 1;

            for (std::size_t j = 0; j < c2; ++j) {
                auto l_lo = std::max<int>(0, j - m2 + 1);
                auto l_hi = std::min(n2 - 1, j) + 1;

                __m128 r1 = _mm_setzero_ps();

                for (std::size_t k = k_lo; k < k_hi; ++k) {
                    for (std::size_t l = l_lo; l + 3 < l_hi; l += 4) {
                        __m128 i1 = _mm_loadu_ps(in + k * n2 + l);
                        __m128 k1 = _mm_loadu_ps(kernel + (m1 - 1 - (i - k)) * m2 + (m2 - 1 - (j - l)));
                        __m128 t1 = _mm_mul_ps(k1, i1);
                        r1        = _mm_add_ps(r1, t1);
                    }
                }

                out[i * c2 + j] = beta * out[i * c2 + j] + detail::mm_hadd_ss(r1);

                double temp = 0.0;

                if ((l_hi - l_lo) % 4 != 0) {
                    auto rem = (l_hi - l_lo) % 4;
                    for (std::size_t k = k_lo; k < k_hi; ++k) {
                        for (std::size_t l = l_hi - rem; l < l_hi; ++l) {
                            temp += in[k * n2 + l] * kernel[(m1 - 1 - (i - k)) * m2 + (m2 - 1 - (j - l))];
                        }
                    }
                }

                out[i * c2 + j] += temp;
            }
        }
    }
}

template <typename T>
void pad_2d_input(const opaque_memory<T, 2>& in, opaque_memory<T, 2>& out, size_t p1, size_t p2) {
    auto in_m = in.memory_start();
    auto out_m = out.memory_start();

    for (std::size_t i = 0; i < in.template dim<0>(); ++i) {
        direct_copy_n(in_m + i * in.template dim<1>(), out_m + (i + p1) * out.template dim<1>() + p2, in.template dim<1>());
    }
}

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename T>
void conv2_valid(const opaque_memory<T, 2>& input, const opaque_memory<T, 2>& kernel, const opaque_memory<T, 2>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    if(cpp_unlikely(p1 || p2)){
        const auto ws_h = input.template dim<0>() + 2 * p1;
        const auto ws_w = input.template dim<1>() + 2 * p2;

        if(ws_h * ws_w * sizeof(T) < max_workspace){
            etl::dyn_matrix<T, 2> workspace(ws_h, ws_w, T(0));
            auto ws_direct = workspace.direct();

            pad_2d_input(input, ws_direct, p1, p2);

            conv2_valid(workspace.direct(), kernel, conv, s1, s2, 0, 0);

            return;
        }
    }

    const auto k2 = kernel.dim(1);

    if(padding_impl){
        constexpr size_t SS = std::is_same<T, float>::value ? 4 : 2;

        if(k2 % SS > 0){
            const auto pad = SS - k2 % SS;

            auto padded_input = common::pad_right(input, pad);
            auto padded_kernel = common::pad_right_flip(kernel, pad);

            conv2_valid_flipped_micro_kernel(
                padded_input.memory_start(), padded_input.template dim<0>(), padded_input.template dim<1>(),
                padded_kernel.memory_start(), padded_kernel.template dim<0>(), padded_kernel.template dim<1>(),
                conv.memory_start(), 0.0, s1, s2, p1, p2);

            return;
        }
    }

    conv2_valid_micro_kernel(
        input.memory_start(), input.template dim<0>(), input.template dim<1>(),
        kernel.memory_start(), kernel.template dim<0>(), kernel.template dim<1>(),
        conv.memory_start(), 0.0, s1, s2, p1, p2);
}

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename T>
void conv2_valid_flipped(const opaque_memory<T, 2>& input, const opaque_memory<T, 2>& kernel, const opaque_memory<T, 2>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    const auto k2 = kernel.dim(1);

    if(padding_impl){
        constexpr size_t SS = std::is_same<T, float>::value ? 4 : 2;

        if(k2 % SS > 0){
            const auto pad = SS - k2 % SS;

            auto padded_input = common::pad_right(input, pad);
            auto padded_kernel = common::pad_right(kernel, pad);

            conv2_valid_flipped_micro_kernel(
                padded_input.memory_start(), padded_input.template dim<0>(), padded_input.template dim<1>(),
                padded_kernel.memory_start(), padded_kernel.template dim<0>(), padded_kernel.template dim<1>(),
                conv.memory_start(), 0.0, s1, s2, p1, p2);

            return;
        }
    }

    conv2_valid_flipped_micro_kernel(
        input.memory_start(), input.template dim<0>(), input.template dim<1>(),
        kernel.memory_start(), kernel.template dim<0>(), kernel.template dim<1>(),
        conv.memory_start(), 0.0, s1, s2, p1, p2);
}

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename T>
void conv2_valid_multi(const opaque_memory<T, 2>& input, const opaque_memory<T, 3>& kernel, const opaque_memory<T, 3>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    const auto K = kernel.template dim<0>();
    const auto k2 = kernel.dim(2);

    if(padding_impl){
        static constexpr size_t SS = std::is_same<T, float>::value ? 4 : 2;

        if(k2 % SS > 0){
            const auto pad = SS - k2 % SS;

            auto padded_input = common::pad_right(input, pad);
            auto padded_kernel = common::pad_right_flip_multi(kernel, pad);

            // TODO Test if it is better to do the padding of the kernel inside each thread

            auto fun_k = [&](const size_t first, const size_t last) {
                for (std::size_t k = first; k < last; ++k) {
                    auto kk = padded_kernel.template dim<1>() * padded_kernel.template dim<2>();
                    auto cc = conv.template dim<1>() * conv.template dim<2>();

                    conv2_valid_flipped_micro_kernel(
                        padded_input.memory_start(), padded_input.template dim<0>(), padded_input.template dim<1>(),
                        padded_kernel.memory_start() + k * kk, padded_kernel.template dim<1>(), padded_kernel.template dim<2>(),
                        conv.memory_start() + k * cc, 0.0, s1, s2, p1, p2);
                }
            };

            dispatch_1d_any(select_parallel(K, 2), fun_k, 0, K);

            return;
        }
    }

    auto fun_k = [&](const size_t first, const size_t last) {
        for (std::size_t k = first; k < last; ++k) {
            auto kk = kernel.template dim<1>() * kernel.template dim<2>();
            auto cc = conv.template dim<1>() * conv.template dim<2>();

            conv2_valid_micro_kernel(
                input.memory_start(), input.template dim<0>(), input.template dim<1>(),
                kernel.memory_start() + k * kk, kernel.template dim<1>(), kernel.template dim<2>(),
                conv.memory_start() + k * cc, 0.0, s1, s2, p1, p2);
        }
    };

    dispatch_1d_any(select_parallel(K, 2), fun_k, 0, K);
}

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename T>
void conv2_valid_multi_flipped(const opaque_memory<T, 2>& input, const opaque_memory<T, 3>& kernel, const opaque_memory<T, 3>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    const auto K = kernel.template dim<0>();
    const auto k2 = kernel.dim(2);

    if(padding_impl){
        constexpr size_t SS = std::is_same<T, float>::value ? 4 : 2;

        if(k2 % SS > 0){
            const auto pad = SS - k2 % SS;

            auto padded_input = common::pad_right(input, pad);
            auto padded_kernel = common::pad_right_multi(kernel, pad);

            // TODO Test if it is better to do the padding of the kernel inside each thread

            auto fun_k = [&](const size_t first, const size_t last) {
                for (std::size_t k = first; k < last; ++k) {
                    auto kk = padded_kernel.template dim<1>() * padded_kernel.template dim<2>();
                    auto cc = conv.template dim<1>() * conv.template dim<2>();

                    conv2_valid_flipped_micro_kernel(
                        padded_input.memory_start(), padded_input.template dim<0>(), padded_input.template dim<1>(),
                        padded_kernel.memory_start() + k * kk, padded_kernel.template dim<1>(), padded_kernel.template dim<2>(),
                        conv.memory_start() + k * cc, 0.0, s1, s2, p1, p2);
                }
            };

            dispatch_1d_any(select_parallel(K, 2), fun_k, 0, K);

            return;
        }
    }

    auto fun_k = [&](const size_t first, const size_t last) {
        for (std::size_t k = first; k < last; ++k) {
            auto kk = kernel.template dim<1>() * kernel.template dim<2>();
            auto cc = conv.template dim<1>() * conv.template dim<2>();

            conv2_valid_flipped_micro_kernel(
                input.memory_start(), input.template dim<0>(), input.template dim<1>(),
                kernel.memory_start() + k * kk, kernel.template dim<1>(), kernel.template dim<2>(),
                conv.memory_start() + k * cc, 0.0, s1, s2, p1, p2);
        }
    };

    dispatch_1d_any(select_parallel(K, 2), fun_k, 0, K);
}

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename T>
void conv2_valid_multi_multi(const opaque_memory<T, 3>& input, const opaque_memory<T, 3>& kernel, const opaque_memory<T, 4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    const auto k2 = kernel.dim(2);
    const auto K  = kernel.template dim<0>();
    const auto N  = input.template dim<0>();
    const auto KN = K * N;

    if(padding_impl){
        static constexpr size_t SS = std::is_same<T, float>::value ? 4 : 2;

        if(k2 % SS > 0){
            const auto pad = SS - k2 % SS;

            auto padded_input = common::pad_right_multi(input, pad);
            auto padded_kernel = common::pad_right_flip_multi(kernel, pad);

            auto fun_kn = [&](const size_t first, const size_t last) {
                for (std::size_t kn = first; kn < last; ++kn) {
                    auto k = kn / N;
                    auto n = kn % N;

                    const auto ii = padded_input.template dim<1>() * padded_input.template dim<2>();
                    const auto kk = padded_kernel.template dim<1>() * padded_kernel.template dim<2>();

                    const auto c_k = conv.template dim<1>() * conv.template dim<2>() * conv.template dim<3>();
                    const auto c_i = conv.template dim<2>() * conv.template dim<3>();

                    conv2_valid_flipped_micro_kernel(
                        padded_input.memory_start() + n * ii, padded_input.template dim<1>(), padded_input.template dim<2>(),
                        padded_kernel.memory_start() + k * kk, padded_kernel.template dim<1>(), padded_kernel.template dim<2>(),
                        conv.memory_start() + k * c_k + n * c_i, 0.0, s1, s2, p1, p2);
                }
            };

            dispatch_1d_any(select_parallel(KN, 2), fun_kn, 0, KN);

            return;
        }
    }

    auto fun_kn = [&](const size_t first, const size_t last) {
        for (std::size_t kn = first; kn < last; ++kn) {
            auto k = kn / N;
            auto n = kn % N;

            const auto ii = input.template dim<1>() * input.template dim<2>();
            const auto kk = kernel.template dim<1>() * kernel.template dim<2>();

            const auto c_k = conv.template dim<1>() * conv.template dim<2>() * conv.template dim<3>();
            const auto c_i = conv.template dim<2>() * conv.template dim<3>();

            conv2_valid_micro_kernel(
                input.memory_start() + n * ii, input.template dim<1>(), input.template dim<2>(),
                kernel.memory_start() + k * kk, kernel.template dim<1>(), kernel.template dim<2>(),
                conv.memory_start() + k * c_k + n * c_i, 0.0, s1, s2, p1, p2);
        }
    };

    dispatch_1d_any(select_parallel(KN, 2), fun_kn, 0, KN);
}

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename T>
void conv2_valid_multi_multi_flipped(const opaque_memory<T, 3>& input, const opaque_memory<T, 3>& kernel, const opaque_memory<T, 4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    const auto k2 = kernel.dim(2);
    const auto K  = kernel.template dim<0>();
    const auto N  = input.template dim<0>();
    const auto KN = K * N;

    if(padding_impl){
        constexpr size_t SS = std::is_same<T, float>::value ? 4 : 2;

        if(k2 % SS > 0){
            const auto pad = SS - k2 % SS;

            auto padded_input = common::pad_right_multi(input, pad);
            auto padded_kernel = common::pad_right_multi(kernel, pad);

            // TODO Test if it is better to do the padding of the kernel inside each thread

            auto fun_kn = [&](const size_t first, const size_t last) {
                for (std::size_t kn = first; kn < last; ++kn) {
                    auto k = kn / N;
                    auto n = kn % N;

                    const auto ii = padded_input.template dim<1>() * padded_input.template dim<2>();
                    const auto kk = padded_kernel.template dim<1>() * padded_kernel.template dim<2>();

                    const auto c_k = conv.template dim<1>() * conv.template dim<2>() * conv.template dim<3>();
                    const auto c_i = conv.template dim<2>() * conv.template dim<3>();

                    conv2_valid_flipped_micro_kernel(
                        padded_input.memory_start() + n * ii, padded_input.template dim<1>(), padded_input.template dim<2>(),
                        padded_kernel.memory_start() + k * kk, padded_kernel.template dim<1>(), padded_kernel.template dim<2>(),
                        conv.memory_start() + k * c_k + n * c_i, 0.0, s1, s2, p1, p2);
                }
            };

            dispatch_1d_any(select_parallel(KN, 2), fun_kn, 0, KN);

            return;
        }
    }

    auto fun_kn = [&](const size_t first, const size_t last) {
        for (std::size_t kn = first; kn < last; ++kn) {
            auto k = kn / N;
            auto n = kn % N;

            const auto ii = input.template dim<1>() * input.template dim<2>();
            const auto kk = kernel.template dim<1>() * kernel.template dim<2>();

            const auto c_k = conv.template dim<1>() * conv.template dim<2>() * conv.template dim<3>();
            const auto c_i = conv.template dim<2>() * conv.template dim<3>();

            conv2_valid_flipped_micro_kernel(
                input.memory_start() + n * ii, input.template dim<1>(), input.template dim<2>(),
                kernel.memory_start() + k * kk, kernel.template dim<1>(), kernel.template dim<2>(),
                conv.memory_start() + k * c_k + n * c_i, 0.0, s1, s2, p1, p2);
        }
    };

    dispatch_1d_any(select_parallel(KN, 2), fun_kn, 0, KN);
}

/*!
 * \brief SSE implementation of a 2D 'same' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_same(const opaque_memory<T, 2>& input, const opaque_memory<T, 2>& kernel, const opaque_memory<T, 2>& conv) {
    conv2_same_micro_kernel(
        input.memory_start(), input.template dim<0>(), input.template dim<1>(),
        kernel.memory_start(), kernel.template dim<0>(), kernel.template dim<1>(),
        conv.memory_start());
}

/*!
 * \brief AVX implementation of a 2D 'same' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_same_flipped(const opaque_memory<T, 2>& input, const opaque_memory<T, 2>& kernel, const opaque_memory<T, 2>& conv) {
    conv2_same_flipped_micro_kernel(
        input.memory_start(), input.template dim<0>(), input.template dim<1>(),
        kernel.memory_start(), kernel.template dim<0>(), kernel.template dim<1>(),
        conv.memory_start());
}

/*!
 * \brief SSE implementation of a 2D 'same' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_same_multi(const opaque_memory<T, 2>& input, const opaque_memory<T, 3>& kernel, const opaque_memory<T, 3>& conv) {
    const auto K = kernel.dim(0);

    auto batch_fun_k = [=](const size_t first, const size_t last) {
        for (std::size_t k = first; k < last; ++k) {
            auto kk = kernel.template dim<1>() * kernel.template dim<2>();
            auto cc = conv.template dim<1>() * conv.template dim<2>();

            conv2_same_micro_kernel(
                input.memory_start(), input.template dim<0>(), input.template dim<1>(),
                kernel.memory_start() + k * kk, kernel.template dim<1>(), kernel.template dim<2>(),
                conv.memory_start() + k * cc);
        }
    };

    dispatch_1d_any(select_parallel(K, 2), batch_fun_k, 0, K);
}

/*!
 * \brief AVX implementation of a 2D 'same' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_same_multi_flipped(const opaque_memory<T, 2>& input, const opaque_memory<T, 3>& kernel, const opaque_memory<T, 3>& conv) {
    const auto K = kernel.dim(0);

    auto batch_fun_k = [=](const size_t first, const size_t last) {
        for (std::size_t k = first; k < last; ++k) {
            auto kk = kernel.template dim<1>() * kernel.template dim<2>();
            auto cc = conv.template dim<1>() * conv.template dim<2>();

            conv2_same_flipped_micro_kernel(
                input.memory_start(), input.template dim<0>(), input.template dim<1>(),
                kernel.memory_start() + k * kk, kernel.template dim<1>(), kernel.template dim<2>(),
                conv.memory_start() + k * cc);
        }
    };

    dispatch_1d_any(select_parallel(K, 2), batch_fun_k, 0, K);
}

/*!
 * \brief SSE implementation of a 2D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_full(const opaque_memory<T, 2>& input, const opaque_memory<T, 2>& kernel, const opaque_memory<T, 2>& conv) {
    conv2_full_micro_kernel(
        input.memory_start(), input.template dim<0>(), input.template dim<1>(),
        kernel.memory_start(), kernel.template dim<0>(), kernel.template dim<1>(),
        conv.memory_start(), 0.0);
}

/*!
 * \brief AVX implementation of a 2D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_full_flipped(const opaque_memory<T, 2>& input, const opaque_memory<T, 2>& kernel, const opaque_memory<T, 2>& conv) {
    conv2_full_flipped_micro_kernel(
        input.memory_start(), input.template dim<0>(), input.template dim<1>(),
        kernel.memory_start(), kernel.template dim<0>(), kernel.template dim<1>(),
        conv.memory_start(), 0.0);
}

/*!
 * \brief SSE implementation of a 2D 'full' convolution C = I * K, with multiple
 * kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_full_multi(const opaque_memory<T, 2>& input, const opaque_memory<T, 3>& kernel, const opaque_memory<T, 3>& conv) {
    const auto K = kernel.dim(0);

    auto batch_fun_k = [=](const size_t first, const size_t last) {
        for (std::size_t k = first; k < last; ++k) {
            auto kk = kernel.template dim<1>() * kernel.template dim<2>();
            auto cc = conv.template dim<1>() * conv.template dim<2>();

            conv2_full_micro_kernel(
                input.memory_start(), input.template dim<0>(), input.template dim<1>(),
                kernel.memory_start() + k * kk, kernel.template dim<1>(), kernel.template dim<2>(),
                conv.memory_start() + k * cc, 0.0);
        }
    };

    dispatch_1d_any(select_parallel(K, 2), batch_fun_k, 0, K);
}

/*!
 * \brief AVX implementation of a 2D 'full' convolution C = I * K, with multiple flipped kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv2_full_multi_flipped(const opaque_memory<T, 2>& input, const opaque_memory<T, 3>& kernel, const opaque_memory<T, 3>& conv) {
    const auto K = kernel.dim(0);

    auto batch_fun_k = [=](const size_t first, const size_t last) {
        for (std::size_t k = first; k < last; ++k) {
            auto kk = kernel.template dim<1>() * kernel.template dim<2>();
            auto cc = conv.template dim<1>() * conv.template dim<2>();

            conv2_full_flipped_micro_kernel(
                input.memory_start(), input.template dim<0>(), input.template dim<1>(),
                kernel.memory_start() + k * kk, kernel.template dim<1>(), kernel.template dim<2>(),
                conv.memory_start() + k * cc, 0.0);
        }
    };

    dispatch_1d_any(select_parallel(K, 2), batch_fun_k, 0, K);
}

/*!
 * \brief SSE implementation of a 4D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    if(kernel.dim(1) > 0){
        const auto N = input.dim(0);  // The number of images
        const auto K = kernel.dim(0); // The number of kernels
        const auto C = input.dim(1);  // The number of channels

        const auto k1 = kernel.dim(2);
        const auto k2 = kernel.dim(3);

        const auto conv_i_inc = conv.dim(1) * conv.dim(2) * conv.dim(3);
        const auto conv_k_inc = conv.dim(2) * conv.dim(3);

        std::fill(conv.memory_start(), conv.memory_end(), 0);

        if(padding_impl){
            static constexpr size_t SS = std::is_same<T, float>::value ? 4 : 2;

            if(k2 % SS > 0){
                const auto pad = SS - k2 % SS;

                auto padded_input = common::pad_right_multi(input, pad);
                auto padded_kernel = common::pad_right_flip_multi(kernel, pad);

                const auto kernel_k_inc = padded_kernel.dim(1) * padded_kernel.dim(2) * padded_kernel.dim(3);
                const auto kernel_c_inc = padded_kernel.dim(2) * padded_kernel.dim(3);

                const auto input_i_inc = padded_input.dim(1) * padded_input.dim(2) * padded_input.dim(3);
                const auto input_c_inc = padded_input.dim(2) * padded_input.dim(3);

                const auto n1 = padded_input.dim(2);
                const auto n2 = padded_input.dim(3);

                auto fun_nk = [&](const size_t first, const size_t last) {
                    for (std::size_t nk = first; nk < last; ++nk) {
                        auto i = nk / K;
                        auto k = nk % K;

                        for (size_t c = 0; c < C; ++c) {
                            auto m_kernel = padded_kernel.memory_start() + k * kernel_k_inc + c * kernel_c_inc;
                            auto m_in     = padded_input.memory_start() + i * input_i_inc + c * input_c_inc;
                            auto m_out    = conv.memory_start() + i * conv_i_inc + k * conv_k_inc;

                            impl::sse::conv2_valid_flipped_micro_kernel(m_in, n1, n2, m_kernel, k1, k2 + pad, m_out, 1.0, s1, s2, p1, p2);
                        }
                    }
                };

                dispatch_1d_any(select_parallel(K * N, 4), fun_nk, 0, K * N);

                return;
            }
        }

        const auto kernel_k_inc = kernel.dim(1) * kernel.dim(2) * kernel.dim(3);
        const auto kernel_c_inc = kernel.dim(2) * kernel.dim(3);

        const auto input_i_inc = input.dim(1) * input.dim(2) * input.dim(3);
        const auto input_c_inc = input.dim(2) * input.dim(3);

        const auto n1 = input.dim(2);
        const auto n2 = input.dim(3);

        auto fun_nk = [&](const size_t first, const size_t last) {
            for (std::size_t nk = first; nk < last; ++nk) {
                auto i = nk / K;
                auto k = nk % K;

                for (size_t c = 0; c < C; ++c) {
                    auto m_kernel = kernel.memory_start() + k * kernel_k_inc + c * kernel_c_inc;
                    auto m_in     = input.memory_start() + i * input_i_inc + c * input_c_inc;
                    auto m_out    = conv.memory_start() + i * conv_i_inc + k * conv_k_inc;

                    impl::sse::conv2_valid_micro_kernel(m_in, n1, n2, m_kernel, k1, k2, m_out, 1.0, s1, s2, p1, p2);
                }
            }
        };

        dispatch_1d_any(select_parallel(K * N, 4), fun_nk, 0, K * N);
    }
}

/*!
 * \brief AVX implementation of a 4D 'valid' convolution C = I * K, with flipped weights
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_flipped(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    if(kernel.dim(1) > 0){
        const auto N = input.dim(0);  // The number of images
        const auto K = kernel.dim(0); // The number of kernels
        const auto C = input.dim(1);  // The number of channels

        const auto k1 = kernel.dim(2);
        const auto k2 = kernel.dim(3);

        auto conv_i_inc = conv.dim(1) * conv.dim(2) * conv.dim(3);
        auto conv_k_inc = conv.dim(2) * conv.dim(3);

        if(padding_impl){
            constexpr size_t SS = std::is_same<T, float>::value ? 4 : 2;

            if(k2 % SS > 0){
                const auto pad = SS - k2 % SS;

                auto padded_input = common::pad_right_multi(input, pad);
                auto padded_kernel = common::pad_right_multi(kernel, pad);

                auto kernel_k_inc = padded_kernel.dim(1) * padded_kernel.dim(2) * padded_kernel.dim(3);
                auto kernel_c_inc = padded_kernel.dim(2) * padded_kernel.dim(3);

                auto input_i_inc = padded_input.dim(1) * padded_input.dim(2) * padded_input.dim(3);
                auto input_c_inc = padded_input.dim(2) * padded_input.dim(3);

                const auto n1 = padded_input.dim(2);
                const auto n2 = padded_input.dim(3);

                const auto* input_mem  = padded_input.memory_start();
                const auto* kernel_mem = padded_kernel.memory_start();
                auto* conv_mem         = conv.memory_start();

                auto fun_nk = [&](const size_t first, const size_t last) {
                    for (std::size_t nk = first; nk < last; ++nk) {
                        auto i = nk / K;
                        auto k = nk % K;

                        //c = 0
                        impl::sse::conv2_valid_flipped_micro_kernel(
                            input_mem + i * input_i_inc, n1, n2,
                            kernel_mem + k * kernel_k_inc, k1, k2 + pad,
                            conv_mem + i * conv_i_inc + k * conv_k_inc, 0.0, s1, s2, p1, p2);

                        // c = [1, C]
                        for (std::size_t c = 1; c < C; ++c) {
                            impl::sse::conv2_valid_flipped_micro_kernel(
                                input_mem + i * input_i_inc + c * input_c_inc, n1, n2,
                                kernel_mem + k * kernel_k_inc + c * kernel_c_inc, k1, k2 + pad,
                                conv_mem + i * conv_i_inc + k * conv_k_inc, 1.0, s1, s2, p1, p2);
                        }
                    }
                };

                dispatch_1d_any(select_parallel(K * N, 4), fun_nk, 0, K * N);

                return;
            }
        }

        auto kernel_k_inc = kernel.dim(1) * kernel.dim(2) * kernel.dim(3);
        auto kernel_c_inc = kernel.dim(2) * kernel.dim(3);

        auto input_i_inc = input.dim(1) * input.dim(2) * input.dim(3);
        auto input_c_inc = input.dim(2) * input.dim(3);

        const auto n1 = input.dim(2);
        const auto n2 = input.dim(3);

        const auto* input_mem  = input.memory_start();
        const auto* kernel_mem = kernel.memory_start();
        auto* conv_mem         = conv.memory_start();

        auto fun_nk = [&](const size_t first, const size_t last) {
            for (std::size_t nk = first; nk < last; ++nk) {
                auto i = nk / K;
                auto k = nk % K;

                //c = 0
                impl::sse::conv2_valid_flipped_micro_kernel(
                    input_mem + i * input_i_inc, n1, n2,
                    kernel_mem + k * kernel_k_inc, k1, k2,
                    conv_mem + i * conv_i_inc + k * conv_k_inc, 0.0, s1, s2, p1, p2);

                // c = [1, C]
                for (std::size_t c = 1; c < C; ++c) {
                    impl::sse::conv2_valid_flipped_micro_kernel(
                        input_mem + i * input_i_inc + c * input_c_inc, n1, n2,
                        kernel_mem + k * kernel_k_inc + c * kernel_c_inc, k1, k2,
                        conv_mem + i * conv_i_inc + k * conv_k_inc, 1.0, s1, s2, p1, p2);
                }
            }
        };

        dispatch_1d_any(select_parallel(K * N, 4), fun_nk, 0, K * N);
    }
}

/*!
 * \brief AVX implementation of a 4D 'valid' convolution C = I * K, where the output are considered to be kernels
 *
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_filter(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    if (input.dim(0) > 0) {
        const auto N = input.dim(0);  // The number of images
        const auto C = input.dim(1);  // The number of channels
        const auto K = kernel.dim(1); // The number of kernels

        const auto k1 = kernel.dim(2);
        const auto k2 = kernel.dim(3);

        const auto conv_k_inc = conv.dim(1) * conv.dim(2) * conv.dim(3);
        const auto conv_c_inc = conv.dim(2) * conv.dim(3);

        if(padding_impl){
            static constexpr size_t SS = std::is_same<T, float>::value ? 4 : 2;

            if(k2 % SS > 0){
                const auto pad = SS - k2 % SS;

                auto padded_input = common::pad_right_multi(input, pad);
                auto padded_kernel = common::pad_right_flip_multi(kernel, pad);

                const auto kernel_i_inc = padded_kernel.dim(1) * padded_kernel.dim(2) * padded_kernel.dim(3);
                const auto kernel_k_inc = padded_kernel.dim(2) * padded_kernel.dim(3);

                const auto input_i_inc = padded_input.dim(1) * padded_input.dim(2) * padded_input.dim(3);
                const auto input_c_inc = padded_input.dim(2) * padded_input.dim(3);

                const auto n1 = padded_input.dim(2);
                const auto n2 = padded_input.dim(3);

                const auto* input_mem  = padded_input.memory_start();
                const auto* kernel_mem = padded_kernel.memory_start();
                auto* conv_mem         = conv.memory_start();

                auto fun_kc = [&](const size_t first, const size_t last) {
                    //i = 0
                    for (std::size_t kc = first; kc < last; ++kc) {
                        auto k = kc / C;
                        auto c = kc % C;

                        impl::sse::conv2_valid_flipped_micro_kernel(
                            input_mem + 0 * input_i_inc + c * input_c_inc, n1, n2,
                            kernel_mem + 0 * kernel_i_inc + k * kernel_k_inc, k1, k2 + pad,
                            conv_mem + k * conv_k_inc + c * conv_c_inc, 0.0, s1, s2, p1, p2);
                    }

                    for (std::size_t i = 1; i < N; ++i) {
                        for (std::size_t kc = first; kc < last; ++kc) {
                            auto k = kc / C;
                            auto c = kc % C;

                            impl::sse::conv2_valid_flipped_micro_kernel(
                                input_mem + i * input_i_inc + c * input_c_inc, n1, n2,
                                kernel_mem + i * kernel_i_inc + k * kernel_k_inc, k1, k2 + pad,
                                conv_mem + k * conv_k_inc + c * conv_c_inc, 1.0, s1, s2, p1, p2);
                        }
                    }
                };

                dispatch_1d_any(select_parallel(K * C, 4), fun_kc, 0, K * C);

                return;
            }
        }

        const auto kernel_i_inc = kernel.dim(1) * kernel.dim(2) * kernel.dim(3);
        const auto kernel_k_inc = kernel.dim(2) * kernel.dim(3);

        const auto input_i_inc = input.dim(1) * input.dim(2) * input.dim(3);
        const auto input_c_inc = input.dim(2) * input.dim(3);

        const auto n1 = input.dim(2);
        const auto n2 = input.dim(3);

        const auto* input_mem  = input.memory_start();
        const auto* kernel_mem = kernel.memory_start();
        auto* conv_mem         = conv.memory_start();

        auto fun_kc = [&](const size_t first, const size_t last) {
            //i = 0
            for (std::size_t kc = first; kc < last; ++kc) {
                auto k = kc / C;
                auto c = kc % C;

                impl::sse::conv2_valid_micro_kernel(
                    input_mem + 0 * input_i_inc + c * input_c_inc, n1, n2,
                    kernel_mem + 0 * kernel_i_inc + k * kernel_k_inc, k1, k2,
                    conv_mem + k * conv_k_inc + c * conv_c_inc, 0.0, s1, s2, p1, p2);
            }

            for (std::size_t i = 1; i < N; ++i) {
                for (std::size_t kc = first; kc < last; ++kc) {
                    auto k = kc / C;
                    auto c = kc % C;

                    impl::sse::conv2_valid_micro_kernel(
                        input_mem + i * input_i_inc + c * input_c_inc, n1, n2,
                        kernel_mem + i * kernel_i_inc + k * kernel_k_inc, k1, k2,
                        conv_mem + k * conv_k_inc + c * conv_c_inc, 1.0, s1, s2, p1, p2);
                }
            }
        };

        dispatch_1d_any(select_parallel(K * C, 4), fun_kc, 0, K * C);
    }
}

/*!
 * \brief SSE implementation of a 4D 'valid' convolution C = I * K, where the output
 * are considered to be kernels, with flipped weights
 *
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_filter_flipped(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    if (input.dim(0) > 0) {
        const auto N = input.dim(0);  // The number of images
        const auto C = input.dim(1);  // The number of channels
        const auto K = kernel.dim(1); // The number of kernels

        const auto k1 = kernel.dim(2);
        const auto k2 = kernel.dim(3);

        const auto conv_k_inc = conv.dim(1) * conv.dim(2) * conv.dim(3);
        const auto conv_c_inc = conv.dim(2) * conv.dim(3);

        if(padding_impl){
            constexpr size_t SS = std::is_same<T, float>::value ? 4 : 2;

            if(k2 % SS > 0){
                const auto pad = SS - k2 % SS;

                auto padded_input = common::pad_right_multi(input, pad);
                auto padded_kernel = common::pad_right_multi(kernel, pad);

                const auto kernel_i_inc = padded_kernel.dim(1) * padded_kernel.dim(2) * padded_kernel.dim(3);
                const auto kernel_k_inc = padded_kernel.dim(2) * padded_kernel.dim(3);

                const auto input_i_inc = padded_input.dim(1) * padded_input.dim(2) * padded_input.dim(3);
                const auto input_c_inc = padded_input.dim(2) * padded_input.dim(3);

                const auto n1 = padded_input.dim(2);
                const auto n2 = padded_input.dim(3);

                const auto* input_mem  = padded_input.memory_start();
                const auto* kernel_mem = padded_kernel.memory_start();
                auto* conv_mem         = conv.memory_start();

                auto fun_kc = [&](const size_t first, const size_t last) {
                    //i = 0
                    for (std::size_t kc = first; kc < last; ++kc) {
                        auto k = kc / C;
                        auto c = kc % C;

                        impl::sse::conv2_valid_flipped_micro_kernel(
                            input_mem + 0 * input_i_inc + c * input_c_inc, n1, n2,
                            kernel_mem + 0 * kernel_i_inc + k * kernel_k_inc, k1, k2 + pad,
                            conv_mem + k * conv_k_inc + c * conv_c_inc, 0.0, s1, s2, p1, p2);
                    }

                    for (std::size_t i = 1; i < N; ++i) {
                        for (std::size_t kc = first; kc < last; ++kc) {
                            auto k = kc / C;
                            auto c = kc % C;

                            impl::sse::conv2_valid_flipped_micro_kernel(
                                input_mem + i * input_i_inc + c * input_c_inc, n1, n2,
                                kernel_mem + i * kernel_i_inc + k * kernel_k_inc, k1, k2 + pad,
                                conv_mem + k * conv_k_inc + c * conv_c_inc, 1.0, s1, s2, p1, p2);
                        }
                    }
                };

                dispatch_1d_any(select_parallel(K * C, 4), fun_kc, 0, K * C);

                return;
            }
        }

        const auto kernel_i_inc = kernel.dim(1) * kernel.dim(2) * kernel.dim(3);
        const auto kernel_k_inc = kernel.dim(2) * kernel.dim(3);

        const auto input_i_inc = input.dim(1) * input.dim(2) * input.dim(3);
        const auto input_c_inc = input.dim(2) * input.dim(3);

        const auto n1 = input.dim(2);
        const auto n2 = input.dim(3);

        const auto* input_mem  = input.memory_start();
        const auto* kernel_mem = kernel.memory_start();
        auto* conv_mem         = conv.memory_start();

        auto fun_kc = [&](const size_t first, const size_t last) {
            //i = 0
            for (std::size_t kc = first; kc < last; ++kc) {
                auto k = kc / C;
                auto c = kc % C;

                impl::sse::conv2_valid_flipped_micro_kernel(
                    input_mem + 0 * input_i_inc + c * input_c_inc, n1, n2,
                    kernel_mem + 0 * kernel_i_inc + k * kernel_k_inc, k1, k2,
                    conv_mem + k * conv_k_inc + c * conv_c_inc, 0.0, s1, s2, p1, p2);
            }

            for (std::size_t i = 1; i < N; ++i) {
                for (std::size_t kc = first; kc < last; ++kc) {
                    auto k = kc / C;
                    auto c = kc % C;

                    impl::sse::conv2_valid_flipped_micro_kernel(
                        input_mem + i * input_i_inc + c * input_c_inc, n1, n2,
                        kernel_mem + i * kernel_i_inc + k * kernel_k_inc, k1, k2,
                        conv_mem + k * conv_k_inc + c * conv_c_inc, 1.0, s1, s2, p1, p2);
                }
            }
        };

        dispatch_1d_any(select_parallel(K * C, 4), fun_kc, 0, K * C);
    }
}

/*!
 * \brief SSE implementation of a 4D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_full(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv) {
    const auto N = input.dim(0);
    const auto K = kernel.dim(0);
    const auto C = kernel.dim(1);

    if (C > 0) {
        auto conv_i_inc = conv.dim(1) * conv.dim(2) * conv.dim(3);
        auto conv_c_inc = conv.dim(2) * conv.dim(3);

        auto kernel_k_inc = kernel.dim(1) * kernel.dim(2) * kernel.dim(3);
        auto kernel_c_inc = kernel.dim(2) * kernel.dim(3);

        auto input_i_inc = input.dim(1) * input.dim(2) * input.dim(3);
        auto input_k_inc = input.dim(2) * input.dim(3);

        const auto k1 = kernel.dim(2);
        const auto k2 = kernel.dim(3);

        etl::dyn_matrix<T, 4> prepared_k(K, C, k1, k2);

        std::copy(kernel.memory_start(), kernel.memory_end(), prepared_k.memory_start());

        prepared_k.deep_fflip_inplace();

        if(N > C){
            auto batch_fun_n = [=](const std::size_t first, const std::size_t last){
                if(last - first){
                    for (std::size_t i = first; i < last; ++i) {
                        // k = 0
                        for (std::size_t c = 0; c < C; ++c) {
                            conv2_full_flipped_micro_kernel(
                                input.memory_start() + i * input_i_inc + 0 * input_k_inc, input.dim(2), input.dim(3),
                                prepared_k.memory_start() + 0 * kernel_k_inc + c * kernel_c_inc, kernel.dim(2), kernel.dim(3),
                                conv.memory_start() + i * conv_i_inc + c * conv_c_inc, 0.0);
                        }

                        for (std::size_t k = 1; k < K; ++k) {
                            for (std::size_t c = 0; c < C; ++c) {
                                conv2_full_flipped_micro_kernel(
                                    input.memory_start() + i * input_i_inc + k * input_k_inc, input.dim(2), input.dim(3),
                                    prepared_k.memory_start() + k * kernel_k_inc + c * kernel_c_inc, kernel.dim(2), kernel.dim(3),
                                    conv.memory_start() + i * conv_i_inc + c * conv_c_inc, 1.0);
                            }
                        }
                    }
                }
            };

            dispatch_1d_any(select_parallel(N, 2), batch_fun_n, 0, N);
        } else {
            auto batch_fun_c = [=](const std::size_t first, const std::size_t last) {
                if (last - first) {
                    for (std::size_t i = 0; i < N; ++i) {
                        // k = 0
                        for (std::size_t c = first; c < last; ++c) {
                            conv2_full_flipped_micro_kernel(
                                input.memory_start() + i * input_i_inc + 0 * input_k_inc, input.dim(2), input.dim(3),
                                prepared_k.memory_start() + 0 * kernel_k_inc + c * kernel_c_inc, kernel.dim(2), kernel.dim(3),
                                conv.memory_start() + i * conv_i_inc + c * conv_c_inc, 0.0);
                        }

                        for (std::size_t k = 1; k < K; ++k) {
                            for (std::size_t c = first; c < last; ++c) {
                                conv2_full_flipped_micro_kernel(
                                    input.memory_start() + i * input_i_inc + k * input_k_inc, input.dim(2), input.dim(3),
                                    prepared_k.memory_start() + k * kernel_k_inc + c * kernel_c_inc, kernel.dim(2), kernel.dim(3),
                                    conv.memory_start() + i * conv_i_inc + c * conv_c_inc, 1.0);
                            }
                        }
                    }
                }
            };

            dispatch_1d_any(select_parallel(C, 2), batch_fun_c, 0, C);
        }
    }
}

/*!
 * \brief SSE implementation of a 4D 'full' convolution C = I * K,
 * with flipped weights
 *
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_full_flipped(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv) {
    const auto N = input.dim(0);
    const auto K = kernel.dim(0);
    const auto C = kernel.dim(1);

    if (C > 0) {
        auto conv_i_inc = conv.dim(1) * conv.dim(2) * conv.dim(3);
        auto conv_c_inc = conv.dim(2) * conv.dim(3);

        auto kernel_k_inc = kernel.dim(1) * kernel.dim(2) * kernel.dim(3);
        auto kernel_c_inc = kernel.dim(2) * kernel.dim(3);

        auto input_i_inc = input.dim(1) * input.dim(2) * input.dim(3);
        auto input_k_inc = input.dim(2) * input.dim(3);

        if(N > C){
            auto batch_fun_n = [=](const std::size_t first, const std::size_t last){
                if(last - first){
                    for (std::size_t i = first; i < last; ++i) {
                        // k = 0
                        for (std::size_t c = 0; c < C; ++c) {
                            conv2_full_flipped_micro_kernel(
                                input.memory_start() + i * input_i_inc + 0 * input_k_inc, input.dim(2), input.dim(3),
                                kernel.memory_start() + 0 * kernel_k_inc + c * kernel_c_inc, kernel.dim(2), kernel.dim(3),
                                conv.memory_start() + i * conv_i_inc + c * conv_c_inc, 0.0);
                        }

                        for (std::size_t k = 1; k < K; ++k) {
                            for (std::size_t c = 0; c < C; ++c) {
                                conv2_full_flipped_micro_kernel(
                                    input.memory_start() + i * input_i_inc + k * input_k_inc, input.dim(2), input.dim(3),
                                    kernel.memory_start() + k * kernel_k_inc + c * kernel_c_inc, kernel.dim(2), kernel.dim(3),
                                    conv.memory_start() + i * conv_i_inc + c * conv_c_inc, 1.0);
                            }
                        }
                    }
                }
            };

            dispatch_1d_any(select_parallel(N, 2), batch_fun_n, 0, N);
        } else {
            auto batch_fun_c = [=](const std::size_t first, const std::size_t last) {
                if (last - first) {
                    for (std::size_t i = 0; i < N; ++i) {
                        // k = 0
                        for (std::size_t c = first; c < last; ++c) {
                            conv2_full_flipped_micro_kernel(
                                input.memory_start() + i * input_i_inc + 0 * input_k_inc, input.dim(2), input.dim(3),
                                kernel.memory_start() + 0 * kernel_k_inc + c * kernel_c_inc, kernel.dim(2), kernel.dim(3),
                                conv.memory_start() + i * conv_i_inc + c * conv_c_inc, 0.0);
                        }

                        for (std::size_t k = 1; k < K; ++k) {
                            for (std::size_t c = first; c < last; ++c) {
                                conv2_full_flipped_micro_kernel(
                                    input.memory_start() + i * input_i_inc + k * input_k_inc, input.dim(2), input.dim(3),
                                    kernel.memory_start() + k * kernel_k_inc + c * kernel_c_inc, kernel.dim(2), kernel.dim(3),
                                    conv.memory_start() + i * conv_i_inc + c * conv_c_inc, 1.0);
                            }
                        }
                    }
                }
            };

            dispatch_1d_any(select_parallel(C, 2), batch_fun_c, 0, C);
        }
    }
}

#else

//COVERAGE_EXCLUDE_BEGIN

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename I, typename K, typename C>
void conv2_valid(const I& input, const K& kernel, C&& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename I, typename K, typename C>
void conv2_valid_flipped(const I& input, const K& kernel, C&& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename I, typename K, typename C>
void conv2_valid_multi(const I& input, const K& kernel, C&& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename I, typename K, typename C>
void conv2_valid_multi_flipped(const I& input, const K& kernel, C&& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename I, typename K, typename C>
void conv2_valid_multi_multi(const I& input, const K& kernel, C&& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 * \param s1 The first dimension stride
 * \param s2 The second dimension stride
 * \param p1 The first dimension padding (left and right)
 * \param p2 The second dimension padding (top and bottom)
 */
template <typename I, typename K, typename C>
void conv2_valid_multi_multi_flipped(const I& input, const K& kernel, C&& conv, size_t s1, size_t s2, size_t p1, size_t p2) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'same' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_same(const I& input, const K& kernel, C&& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'same' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_same_flipped(const I& input, const K& kernel, C&& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'same' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_same_multi(const I& input, const K& kernel, C&& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'same' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_same_multi_flipped(const I& input, const K& kernel, C&& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_full(const I& input, const K& kernel, C&& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_full_flipped(const I& input, const K& kernel, C&& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'full' convolution C = I * K, with multiple
 * kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_full_multi(const I& input, const K& kernel, C&& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 2D 'full' convolution C = I * K, with multiple
 * flipped kernels
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename I, typename K, typename C>
void conv2_full_multi_flipped(const I& input, const K& kernel, C&& conv) {
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 4D 'valid' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv, size_t s1, size_t s2, size_t p1, size_t p2){
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 4D 'valid' convolution C = I * K, with flipped weights
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_flipped(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv, size_t s1, size_t s2, size_t p1, size_t p2){
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 4D 'valid' convolution C = I * K, where the output are considered to be kernels
 *
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_filter(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv, size_t s1, size_t s2, size_t p1, size_t p2){
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 4D 'valid' convolution C = I * K, where the output
 * are considered to be kernels, with flipped weights
 *
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_valid_filter_flipped(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv, size_t s1, size_t s2, size_t p1, size_t p2){
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unused(s1);
    cpp_unused(s2);
    cpp_unused(p1);
    cpp_unused(p2);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 4D 'full' convolution C = I * K
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_full(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv){
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("SSE not available/enabled");
}

/*!
 * \brief SSE implementation of a 4D 'full' convolution C = I * K,
 * with flipped weights
 *
 * \param input The input matrix
 * \param kernel The kernel matrix
 * \param conv The output matrix
 */
template <typename T>
void conv4_full_flipped(const opaque_memory<T, 4>& input, const opaque_memory<T, 4>& kernel, const opaque_memory<T, 4>& conv){
    cpp_unused(input);
    cpp_unused(kernel);
    cpp_unused(conv);
    cpp_unreachable("SSE not available/enabled");
}

//COVERAGE_EXCLUDE_END

#endif

} //end of namespace sse
} //end of namespace impl
} //end of namespace etl
