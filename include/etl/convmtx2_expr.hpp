//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_CONVMTX2_EXPR_HPP
#define ETL_CONVMTX2_EXPR_HPP

#include <algorithm>

#include "cpp_utils/assert.hpp"
#include "cpp_utils/tmp.hpp"

#include "traits_lite.hpp"

//Get the implementations
//TODO #include "impl/convmtx2.hpp"

namespace etl {

template<typename T, std::size_t K1, std::size_t K2, template<typename...> class Impl>
struct basic_convmtx2_expr {
    static_assert(K1 > 0, "K1 must be greater than 0");
    static_assert(K2 > 0, "K2 must be greater than 0");

    using this_type = basic_convmtx2_expr<T, K1, K2, Impl>;

    //TODO Restore building of fast matrix impl

    template<typename A, class Enable = void>
    struct result_type_builder {
        using type = dyn_matrix<T, 2>;
    };

    template<typename A>
    using result_type = typename result_type_builder<A>::type;

    template<typename A>
    static result_type<A>* allocate(A&& a){
        auto c_height = (etl::dim<0>(a) + K1 - 1) * (etl::dim<1>(a) + K2 - 1);
        auto c_width = K1 * K2;
        return new result_type<A>(c_height, c_width);
    }

    template<typename A, typename B, typename C>
    static void apply(A&& a, C&& c){
        static_assert(all_etl_expr<A,C>::value, "convmtx2 only supported for ETL expressions");
        static_assert(decay_traits<A>::dimensions == 2 && decay_traits<C>::dimensions == 2, "convmtx2 needs 2D matrices");

        Impl<decltype(make_temporary(std::forward<A>(a))), C>::apply<K1, K2>(
            make_temporary(std::forward<A>(a)),
            std::forward<C>(c));
    }

    static std::string desc() noexcept {
        return "convmtx2";
    }

    template<typename A>
    static std::size_t dim(const A& a, std::size_t d){
        if(d == 0){
            return (etl::dim<0>(a) + K1 - 1) * (etl::dim<1>(a) + K2 - 1);
        } else {
            return K1 * K2;
        }
    }

    template<typename A>
    static std::size_t size(const A& a){
        return (K1 * K2) * ((etl::dim<0>(a) + K1 - 1) * (etl::dim<1>(a) + K2 - 1));
    }

    static constexpr std::size_t dimensions(){
        return 2;
    }
};

//TODO Move to impl header
namespace detail {

template<typename A, typename M>
struct convmtx2_direct {
    template<std::size_t K1, std::size_t K2>
    void apply(A&& sub, M& m){
        const auto i1 = etl::dim<0>(sub);
        const auto i2 = etl::dim<1>(sub);

        const auto c_height = (i1 + K1 - 1) * (i2 + K2 - 1);
        constexpr const auto c_width = K1 * K2;

        const auto max_fill = c_height - ((i1 + K1 - 1) * ((c_width - 1) / K1) + (c_width - 1) % K1);
        const auto inner_paddings = max_fill - (i1 * i2);
        const auto inner_padding = inner_paddings / (i2 - 1);

        m = 0;

        for(std::size_t j = 0; j < c_width; ++j){
            auto top_padding = (i1 + K1 - 1) * (j / K1) + j % K1;
            auto bottom_padding = top_padding + (i1 * i2) + inner_paddings;

            for(std::size_t i = top_padding; i < bottom_padding; ++i){
                auto inner = i - top_padding;
                auto block = inner / (i1 + inner_padding);
                auto col = inner % (i1 + inner_padding);

                if(col < i1){
                    m(i, j) = sub(col, block);
                }
            }
        }
    }
};

}

//Direct convmtx2

template<typename T, std::size_t K1, std::size_t K2>
using direct_convmtx2_expr = basic_convmtx2_expr<T, K1, K2, detail::convmtx2_direct>;

} //end of namespace etl

#endif