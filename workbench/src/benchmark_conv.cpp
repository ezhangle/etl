//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#define CPM_LIB
#include "benchmark.hpp"

//1D-Convolution benchmarks with large-kernel
CPM_BENCH() {
    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000), VALUES_POLICY(500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000)),
        "r = conv_1d_full(a,b)(large)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dvec(d1), dvec(d2), dvec(d1 + d2 - 1)); },
        [](dvec& a, dvec& b, dvec& r){ r = etl::conv_1d_full(a, b); }
        );

    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000), VALUES_POLICY(500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000)),
        "r = conv_1d_same(a,b)(large)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dvec(d1), dvec(d2), dvec(d1)); },
        [](dvec& a, dvec& b, dvec& r){ r = etl::conv_1d_same(a, b); }
        );

    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000), VALUES_POLICY(500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000)),
        "r = conv_1d_valid(a,b)(large)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dvec(d1), dvec(d2), dvec(d1 - d2 + 1)); },
        [](dvec& a, dvec& b, dvec& r){ r = etl::conv_1d_valid(a, b); }
        );
}

//1D-Convolution benchmarks with small-kernel
CPM_BENCH() {
    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000), VALUES_POLICY(100, 200, 300, 400, 500, 600, 700, 800, 900, 1000)),
        "r = conv_1d_full(a,b)(small)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dvec(d1), dvec(d2), dvec(d1 + d2 - 1)); },
        [](dvec& a, dvec& b, dvec& r){ r = etl::conv_1d_full(a, b); }
        );

    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000), VALUES_POLICY(100, 200, 300, 400, 500, 600, 700, 800, 900, 1000)),
        "r = conv_1d_same(a,b)(small)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dvec(d1), dvec(d2), dvec(d1)); },
        [](dvec& a, dvec& b, dvec& r){ r = etl::conv_1d_same(a, b); }
        );

    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000), VALUES_POLICY(100, 200, 300, 400, 500, 600, 700, 800, 900, 1000)),
        "r = conv_1d_valid(a,b)(small)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dvec(d1), dvec(d2), dvec(d1 - d2 + 1)); },
        [](dvec& a, dvec& b, dvec& r){ r = etl::conv_1d_valid(a, b); }
        );
}

//2D-Convolution benchmarks with large-kernel
CPM_BENCH() {
    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(100, 110, 120, 130, 140, 150, 160, 170, 180), VALUES_POLICY(50, 55, 60, 65, 70, 75, 80, 85, 90)),
        "r = conv_2d_full(a,b)(large)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1, d1), dmat(d2, d2), dmat(d1 + d2 - 1, d1 + d2 - 1)); },
        [](dmat& a, dmat& b, dmat& r){ r = etl::conv_2d_full(a, b); }
        );

    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(100, 110, 120, 130, 140, 150, 160, 170, 180), VALUES_POLICY(50, 55, 60, 65, 70, 75, 80, 85, 90)),
        "r = conv_2d_same(a,b)(large)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1, d1), dmat(d2, d2), dmat(d1, d1)); },
        [](dmat& a, dmat& b, dmat& r){ r = etl::conv_2d_same(a, b); }
        );

    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(100, 110, 120, 130, 140, 150, 160, 170, 180), VALUES_POLICY(50, 55, 60, 65, 70, 75, 80, 85, 90)),
        "r = conv_2d_valid(a,b)(large)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1, d1), dmat(d2, d2), dmat(d1 - d2 + 1, d1 - d2 + 1)); },
        [](dmat& a, dmat& b, dmat& r){ r = etl::conv_2d_valid(a, b); }
        );
}

//2D-Convolution benchmarks with small-kernel
CPM_BENCH() {
    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(100, 150, 200, 250, 300, 350, 400), VALUES_POLICY(10, 15, 20, 25, 30, 35, 40)),
        "r = conv_2d_full(a,b)(small)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1, d1), dmat(d2, d2), dmat(d1 + d2 - 1, d1 + d2 - 1)); },
        [](dmat& a, dmat& b, dmat& r){ r = etl::conv_2d_full(a, b); }
        );

    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(100, 150, 200, 250, 300, 350, 400), VALUES_POLICY(10, 15, 20, 25, 30, 35, 40)),
        "r = conv_2d_same(a,b)(small)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1, d1), dmat(d2, d2), dmat(d1, d1)); },
        [](dmat& a, dmat& b, dmat& r){ r = etl::conv_2d_same(a, b); }
        );

    CPM_TWO_PASS_NS_P(
        NARY_POLICY(VALUES_POLICY(100, 150, 200, 250, 300, 350, 400), VALUES_POLICY(10, 15, 20, 25, 30, 35, 40)),
        "r = conv_2d_valid(a,b)(small)",
        [](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1, d1), dmat(d2, d2), dmat(d1 - d2 + 1, d1 - d2 + 1)); },
        [](dmat& a, dmat& b, dmat& r){ r = etl::conv_2d_valid(a, b); }
        );
}

CPM_DIRECT_BENCH_TWO_PASS_P(
    NARY_POLICY(VALUES_POLICY(16, 16, 32, 32, 64, 64), VALUES_POLICY(4, 8, 8, 16, 16, 24)),
    "convmtx2",
    [](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1, d1), dmat((d1 + d2 - 1)*(d1 + d2 - 1), d2 * d2)); }, [](std::size_t /*d1*/, std::size_t d2, dmat& a, dmat& b){ b = etl::convmtx2(a, d2, d2); }
)

CPM_DIRECT_BENCH_TWO_PASS_P(
    NARY_POLICY(VALUES_POLICY(16, 16, 32, 32, 64, 64, 128), VALUES_POLICY(4, 8, 8, 16, 16, 32, 32)),
    "convmtx2_t",
    [](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1, d1), dmat((d1 + d2 - 1)*(d1 + d2 - 1), d2 * d2)); },
    [](std::size_t /*d1*/, std::size_t d2, dmat& a, dmat& b){ etl::convmtx2_direct_t(b, a, d2, d2); }
)

CPM_DIRECT_BENCH_TWO_PASS_P(
    NARY_POLICY(VALUES_POLICY(16, 16, 32, 32, 64, 64, 128), VALUES_POLICY(4, 8, 8, 16, 16, 32, 32)),
    "im2col_direct",
    [](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1, d1), dmat(d2 * d2, (d1 - d2 + 1)*(d1 - d2 + 1))); },
    [](std::size_t /*d1*/, std::size_t d2, dmat& a, dmat& b){ etl::im2col_direct(b, a, d2, d2); }
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("sconv1_valid", conv_1d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(svec(d1), svec(d2), svec(d1 - d2 + 1)); }),
    CPM_SECTION_FUNCTOR("default", [](svec& a, svec& b, svec& r){ r = etl::conv_1d_valid(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](svec& a, svec& b, svec& r){ etl::impl::standard::conv1_valid(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](svec& a, svec& b, svec& r){ etl::impl::sse::conv1_valid(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](svec& a, svec& b, svec& r){ etl::impl::avx::conv1_valid(a, b, r); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("dconv1_valid", conv_1d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(dvec(d1), dvec(d2), dvec(d1 - d2 + 1)); }),
    CPM_SECTION_FUNCTOR("default", [](dvec& a, dvec& b, dvec& r){ r = etl::conv_1d_valid(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](dvec& a, dvec& b, dvec& r){ etl::impl::standard::conv1_valid(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](dvec& a, dvec& b, dvec& r){ etl::impl::sse::conv1_valid(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](dvec& a, dvec& b, dvec& r){ etl::impl::avx::conv1_valid(a, b, r); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("sconv1_same", conv_1d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(svec(d1), svec(d2), svec(d1)); }),
    CPM_SECTION_FUNCTOR("default", [](svec& a, svec& b, svec& r){ r = etl::conv_1d_same(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](svec& a, svec& b, svec& r){ etl::impl::standard::conv1_same(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](svec& a, svec& b, svec& r){ etl::impl::sse::conv1_same(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](svec& a, svec& b, svec& r){ etl::impl::avx::conv1_same(a, b, r); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("dconv1_same", conv_1d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(dvec(d1), dvec(d2), dvec(d1)); }),
    CPM_SECTION_FUNCTOR("default", [](dvec& a, dvec& b, dvec& r){ r = etl::conv_1d_same(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](dvec& a, dvec& b, dvec& r){ etl::impl::standard::conv1_same(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](dvec& a, dvec& b, dvec& r){ etl::impl::sse::conv1_same(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](dvec& a, dvec& b, dvec& r){ etl::impl::avx::conv1_same(a, b, r); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("sconv1_full", conv_1d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(svec(d1), svec(d2), svec(d1 + d2 - 1)); }),
    CPM_SECTION_FUNCTOR("default", [](svec& a, svec& b, svec& r){ r = etl::conv_1d_full(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](svec& a, svec& b, svec& r){ etl::impl::standard::conv1_full(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](svec& a, svec& b, svec& r){ etl::impl::sse::conv1_full(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](svec& a, svec& b, svec& r){ etl::impl::avx::conv1_full(a, b, r); })
    ,CPM_SECTION_FUNCTOR("fft", [](svec& a, svec& b, svec& r){ r = etl::fft_conv_1d_full(a, b); })
     MC_SECTION_FUNCTOR("mmul", [](svec& a, svec& b, svec& r){ etl::impl::reduc::conv1_full(a, b, r); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("dconv1_full", conv_1d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(dvec(d1), dvec(d2), dvec(d1 + d2 - 1)); }),
    CPM_SECTION_FUNCTOR("default", [](dvec& a, dvec& b, dvec& r){ r = etl::conv_1d_full(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](dvec& a, dvec& b, dvec& r){ etl::impl::standard::conv1_full(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](dvec& a, dvec& b, dvec& r){ etl::impl::sse::conv1_full(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](dvec& a, dvec& b, dvec& r){ etl::impl::avx::conv1_full(a, b, r); })
    ,CPM_SECTION_FUNCTOR("fft", [](dvec& a, dvec& b, dvec& r){ r = etl::fft_conv_1d_full(a, b); })
     MC_SECTION_FUNCTOR("mmul", [](dvec& a, dvec& b, dvec& r){ etl::impl::reduc::conv1_full(a, b, r); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("sconv2_valid", conv_2d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(smat(d1,d1), smat(d2,d2), smat(d1 - d2 + 1, d1 - d2 + 1)); }),
    CPM_SECTION_FUNCTOR("default", [](smat& a, smat& b, smat& r){ r = etl::conv_2d_valid(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](smat& a, smat& b, smat& r){ etl::impl::standard::conv2_valid(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](smat& a, smat& b, smat& r){ etl::impl::sse::conv2_valid(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](smat& a, smat& b, smat& r){ etl::impl::avx::conv2_valid(a, b, r); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("dconv2_valid", conv_2d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1,d1), dmat(d2,d2), dmat(d1 - d2 + 1, d1 - d2 + 1)); }),
    CPM_SECTION_FUNCTOR("default", [](dmat& a, dmat& b, dmat& r){ r = etl::conv_2d_valid(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](dmat& a, dmat& b, dmat& r){ etl::impl::standard::conv2_valid(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](dmat& a, dmat& b, dmat& r){ etl::impl::sse::conv2_valid(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](dmat& a, dmat& b, dmat& r){ etl::impl::avx::conv2_valid(a, b, r); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("sconv2_same", conv_2d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(smat(d1,d1), smat(d2,d2), smat(d1,d1)); }),
    CPM_SECTION_FUNCTOR("default", [](smat& a, smat& b, smat& r){ r = etl::conv_2d_same(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](smat& a, smat& b, smat& r){ etl::impl::standard::conv2_same(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](smat& a, smat& b, smat& r){ etl::impl::sse::conv2_same(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](smat& a, smat& b, smat& r){ etl::impl::avx::conv2_same(a, b, r); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("dconv2_same", conv_2d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1,d1), dmat(d2,d2), dmat(d1,d1)); }),
    CPM_SECTION_FUNCTOR("default", [](dmat& a, dmat& b, dmat& r){ r = etl::conv_2d_same(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](dmat& a, dmat& b, dmat& r){ etl::impl::standard::conv2_same(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](dmat& a, dmat& b, dmat& r){ etl::impl::sse::conv2_same(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](dmat& a, dmat& b, dmat& r){ etl::impl::avx::conv2_same(a, b, r); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("sconv2_full", conv_2d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(smat(d1,d1), smat(d2,d2), smat(d1 + d2 - 1, d1 + d2 - 1)); }),
    CPM_SECTION_FUNCTOR("default", [](smat& a, smat& b, smat& r){ r = etl::conv_2d_full(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](smat& a, smat& b, smat& r){ etl::impl::standard::conv2_full(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](smat& a, smat& b, smat& r){ etl::impl::sse::conv2_full(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](smat& a, smat& b, smat& r){ etl::impl::avx::conv2_full(a, b, r); })
    ,CPM_SECTION_FUNCTOR("fft", [](smat& a, smat& b, smat& r){ r = etl::fft_conv_2d_full(a, b); })
     MC_SECTION_FUNCTOR("mmul", [](smat& a, smat& b, smat& r){ etl::impl::reduc::conv2_full(a, b, r); })
)

CPM_DIRECT_SECTION_TWO_PASS_NS_P("dconv2_full", conv_2d_large_policy,
    CPM_SECTION_INIT([](std::size_t d1, std::size_t d2){ return std::make_tuple(dmat(d1,d1), dmat(d2,d2), dmat(d1 + d2 - 1, d1 + d2 - 1)); }),
    CPM_SECTION_FUNCTOR("default", [](dmat& a, dmat& b, dmat& r){ r = etl::conv_2d_full(a, b); }),
    CPM_SECTION_FUNCTOR("std", [](dmat& a, dmat& b, dmat& r){ etl::impl::standard::conv2_full(a, b, r); })
    SSE_SECTION_FUNCTOR("sse", [](dmat& a, dmat& b, dmat& r){ etl::impl::sse::conv2_full(a, b, r); })
    AVX_SECTION_FUNCTOR("avx", [](dmat& a, dmat& b, dmat& r){ etl::impl::avx::conv2_full(a, b, r); })
    ,CPM_SECTION_FUNCTOR("fft", [](dmat& a, dmat& b, dmat& r){ r = etl::fft_conv_2d_full(a, b); })
     MC_SECTION_FUNCTOR("mmul", [](dmat& a, dmat& b, dmat& r){ etl::impl::reduc::conv2_full(a, b, r); })
)