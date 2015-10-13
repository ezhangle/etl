//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#define CPM_LIB
#include "benchmark.hpp"

CPM_DIRECT_SECTION_TWO_PASS_NS_P("A * B (s)", square_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(smat(d1,d2), smat(d1,d2), smat(d1, d2)); }),
    CPM_SECTION_FUNCTOR("default", [](smat& a, smat& b, smat& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](smat& a, smat& b, smat& c){ etl::impl::standard::mm_mul(a, b, c); }),
    CPM_SECTION_FUNCTOR("eblas", [](smat& a, smat& b, smat& c){ etl::impl::eblas::gemm(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](smat& a, smat& b, smat& c){ etl::impl::blas::gemm(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](smat& a, smat& b, smat& c){ etl::impl::cublas::gemm(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("A * B (cm/s)", square_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(smat_cm(d1,d2), smat_cm(d1,d2), smat_cm(d1, d2)); }),
    CPM_SECTION_FUNCTOR("default", [](smat_cm& a, smat_cm& b, smat_cm& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](smat_cm& a, smat_cm& b, smat_cm& c){ etl::impl::standard::mm_mul(a, b, c); }),
    CPM_SECTION_FUNCTOR("eblas", [](smat_cm& a, smat_cm& b, smat_cm& c){ etl::impl::eblas::gemm(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](smat_cm& a, smat_cm& b, smat_cm& c){ etl::impl::blas::gemm(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](smat_cm& a, smat_cm& b, smat_cm& c){ etl::impl::cublas::gemm(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("A * B (d)", square_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1,d2), dmat(d1,d2), dmat(d1, d2)); }),
    CPM_SECTION_FUNCTOR("default", [](dmat& a, dmat& b, dmat& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](dmat& a, dmat& b, dmat& c){ etl::impl::standard::mm_mul(a, b, c); }),
    CPM_SECTION_FUNCTOR("eblas", [](dmat& a, dmat& b, dmat& c){ etl::impl::eblas::gemm(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](dmat& a, dmat& b, dmat& c){ etl::impl::blas::gemm(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](dmat& a, dmat& b, dmat& c){ etl::impl::cublas::gemm(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("A * B (c)", small_square_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(cmat(d1,d2), cmat(d1,d2), cmat(d1, d2)); }),
    CPM_SECTION_FUNCTOR("default", [](cmat& a, cmat& b, cmat& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](cmat& a, cmat& b, cmat& c){ etl::impl::standard::mm_mul(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](cmat& a, cmat& b, cmat& c){ etl::impl::blas::gemm(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](cmat& a, cmat& b, cmat& c){ etl::impl::cublas::gemm(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("A * B (z)", small_square_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(zmat(d1,d2), zmat(d1,d2), zmat(d1, d2)); }),
    CPM_SECTION_FUNCTOR("default", [](zmat& a, zmat& b, zmat& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](zmat& a, zmat& b, zmat& c){ etl::impl::standard::mm_mul(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](zmat& a, zmat& b, zmat& c){ etl::impl::blas::gemm(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](zmat& a, zmat& b, zmat& c){ etl::impl::cublas::gemm(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("A * x (s)", gemv_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(smat(d1,d2), svec(d2), svec(d1)); }),
    CPM_SECTION_FUNCTOR("default", [](smat& a, svec& b, svec& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](smat& a, svec& b, svec& c){ etl::impl::standard::mv_mul(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](smat& a, svec& b, svec& c){ etl::impl::blas::gemv(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](smat& a, svec& b, svec& c){ etl::impl::cublas::gemv(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("A * x (d)", gemv_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1,d2), dvec(d2), dvec(d1)); }),
    CPM_SECTION_FUNCTOR("default", [](dmat& a, dvec& b, dvec& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](dmat& a, dvec& b, dvec& c){ etl::impl::standard::mv_mul(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](dmat& a, dvec& b, dvec& c){ etl::impl::blas::gemv(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](dmat& a, dvec& b, dvec& c){ etl::impl::cublas::gemv(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("A * x (c)", gemv_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(cmat(d1,d2), cvec(d2), cvec(d1)); }),
    CPM_SECTION_FUNCTOR("default", [](cmat& a, cvec& b, cvec& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](cmat& a, cvec& b, cvec& c){ etl::impl::standard::mv_mul(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](cmat& a, cvec& b, cvec& c){ etl::impl::blas::gemv(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](cmat& a, cvec& b, cvec& c){ etl::impl::cublas::gemv(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("A * x (z)", gemv_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(zmat(d1,d2), zvec(d2), zvec(d1)); }),
    CPM_SECTION_FUNCTOR("default", [](zmat& a, zvec& b, zvec& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](zmat& a, zvec& b, zvec& c){ etl::impl::standard::mv_mul(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](zmat& a, zvec& b, zvec& c){ etl::impl::blas::gemv(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](zmat& a, zvec& b, zvec& c){ etl::impl::cublas::gemv(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("x * A (s)", gemv_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(svec(d1), smat(d1,d2), svec(d2)); }),
    CPM_SECTION_FUNCTOR("default", [](svec& a, smat& b, svec& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](svec& a, smat& b, svec& c){ etl::impl::standard::vm_mul(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](svec& a, smat& b, svec& c){ etl::impl::blas::gevm(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](svec& a, smat& b, svec& c){ etl::impl::cublas::gevm(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("x * A (d)", gemv_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(dvec(d1), dmat(d1,d2), dvec(d2)); }),
    CPM_SECTION_FUNCTOR("default", [](dvec& a, dmat& b, dvec& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](dvec& a, dmat& b, dvec& c){ etl::impl::standard::vm_mul(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](dvec& a, dmat& b, dvec& c){ etl::impl::blas::gevm(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](dvec& a, dmat& b, dvec& c){ etl::impl::cublas::gevm(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("x * A (c)", gemv_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(cvec(d1), cmat(d1,d2), cvec(d2)); }),
    CPM_SECTION_FUNCTOR("default", [](cvec& a, cmat& b, cvec& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](cvec& a, cmat& b, cvec& c){ etl::impl::standard::vm_mul(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](cvec& a, cmat& b, cvec& c){ etl::impl::blas::gevm(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](cvec& a, cmat& b, cvec& c){ etl::impl::cublas::gevm(a, b, c); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("x * A (z)", gemv_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(zvec(d1), zmat(d1,d2), zvec(d2)); }),
    CPM_SECTION_FUNCTOR("default", [](zvec& a, zmat& b, zvec& c){ c = a * b; }),
    CPM_SECTION_FUNCTOR("std", [](zvec& a, zmat& b, zvec& c){ etl::impl::standard::vm_mul(a, b, c); })
    BLAS_SECTION_FUNCTOR("blas", [](zvec& a, zmat& b, zvec& c){ etl::impl::blas::gevm(a, b, c); })
    CUBLAS_SECTION_FUNCTOR("cublas", [](zvec& a, zmat& b, zvec& c){ etl::impl::cublas::gevm(a, b, c); })
)

CPM_BENCH(){
    CPM_TWO_PASS_NS_P(
        VALUES_POLICY(10, 50, 100, 500, 1000, 2000, 3000, 4000, 5000),
        "r = A * (a + b)",
        [](std::size_t d){ return std::make_tuple(dmat(d,d), dvec(d), dvec(d), dvec(d)); },
        [](dmat& A, dvec& a, dvec& b, dvec& r){ r = A * (a + b); }
        );
}

CPM_BENCH(){
    CPM_TWO_PASS_NS_P(
        VALUES_POLICY(10, 50, 100, 500, 1000, 2000, 3000, 4000, 5000),
        "r = A * (a + b + c)",
        [](std::size_t d){ return std::make_tuple(dmat(d,d), dvec(d), dvec(d), dvec(d), dvec(d)); },
        [](dmat& A, dvec& a, dvec& b, dvec& c, dvec& r){ r = A * (a + b + c); }
        );
}

CPM_BENCH(){
    CPM_TWO_PASS_NS_P(
        square_policy,
        "R = A * (B + C)",
        [](std::size_t d1,std::size_t d2){ return std::make_tuple(dmat(d1,d2), dmat(d1,d2), dmat(d1,d2), dmat(d1,d2)); },
        [](dmat& A, dmat& B, dmat& C, dmat& R){ R = A * (B + C); }
        );
}

CPM_BENCH(){
    CPM_TWO_PASS_NS_P(
        square_policy,
        "R = A * (B + C + D)",
        [](std::size_t d1,std::size_t d2){ return std::make_tuple(dmat(d1,d2), dmat(d1,d2), dmat(d1,d2), dmat(d1,d2), dmat(d1,d2)); },
        [](dmat& A, dmat& B, dmat& C, dmat& D, dmat& R){ R = A * (B + C + D); }
        );
}

CPM_BENCH(){
    CPM_TWO_PASS_NS_P(
        square_policy,
        "R = (A + B) * (C + D)",
        [](std::size_t d1,std::size_t d2){ return std::make_tuple(dmat(d1,d2), dmat(d1,d2), dmat(d1,d2), dmat(d1,d2), dmat(d1,d2)); },
        [](dmat& A, dmat& B, dmat& C, dmat& D, dmat& R){ R = (A + B) * (C + D); }
        );
}

CPM_BENCH(){
    CPM_TWO_PASS_NS_P(
        square_policy,
        "r = a * (A + B - C)",
        [](std::size_t d1,std::size_t d2){ return std::make_tuple(dvec(d1), dmat(d1,d2), dmat(d1,d2), dmat(d1,d2), dvec(d2)); },
        [](dvec& a, dmat& A, dmat& B, dmat& C, dvec& r){ r = a * (A + B - C); }
        );
}

CPM_BENCH(){
    CPM_TWO_PASS_NS_P(
        square_policy,
        "r = a * (A * B)",
        [](std::size_t d1,std::size_t d2){ return std::make_tuple(dvec(d1), dmat(d1,d2), dmat(d1,d2), dvec(d2)); },
        [](dvec& a, dmat& A, dmat& B, dvec& r){ r = a * (A * B); }
        );
}

CPM_BENCH(){
    CPM_TWO_PASS_NS_P(
        square_policy,
        "r = a * A * B",
        [](std::size_t d1,std::size_t d2){ return std::make_tuple(dvec(d1), dmat(d1,d2), dmat(d1,d2), dvec(d2)); },
        [](dvec& a, dmat& A, dmat& B, dvec& r){ r = a * (A * B); }
        );
}