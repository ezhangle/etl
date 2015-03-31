//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_IMPL_BLAS_GEMM_HPP
#define ETL_IMPL_BLAS_GEMM_HPP

#include "../../config.hpp"

#ifdef ETL_BLAS_MODE

extern "C"
{
#include "cblas.h"
}

#endif

namespace etl {

namespace impl {

namespace blas {

#ifdef ETL_BLAS_MODE

template<typename A, typename B, typename C>
void dgemm(A&& a, B&& b, C&& c){
    cblas_dgemm(
        CblasRowMajor, CblasNoTrans, CblasNoTrans, 
        etl::rows(a), etl::columns(b), etl::columns(a),
        1.0,
        a.memory_start(), etl::dim<1>(a),
        b.memory_start(), etl::dim<1>(b),
        0.0,
        c.memory_start(), etl::dim<1>(c)
    );
};

template<typename A, typename B, typename C>
void sgemm(A&& a, B&& b, C&& c){
    cblas_sgemm(
        CblasRowMajor, CblasNoTrans, CblasNoTrans, 
        etl::rows(a), etl::columns(b), etl::columns(a),
        1.0,
        a.memory_start(), etl::dim<1>(a),
        b.memory_start(), etl::dim<1>(b),
        0.0,
        c.memory_start(), etl::dim<1>(c)
    );
};

#else

template<typename A, typename B, typename C>
void dgemm(A&& a, B&& b, C&& c);

template<typename A, typename B, typename C>
void sgemm(A&& a, B&& b, C&& c);

#endif

} //end of namespace blas

} //end of namespace impl

} //end of namespace etl

#endif