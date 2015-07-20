//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <algorithm>

#include "cpp_utils/assert.hpp"
#include "cpp_utils/tmp.hpp"

#include "traits_lite.hpp"

//Get the implementations
#include "impl/pooling.hpp"

namespace etl {

template<typename T, std::size_t C1, std::size_t C2, template<typename...> class Impl>
struct basic_upsample_2d_expr {
    static_assert(C1 > 0, "C1 must be greater than 0");
    static_assert(C2 > 0, "C2 must be greater than 0");

    using this_type = basic_upsample_2d_expr<T, C1, C2, Impl>;

    template<typename A, typename B, std::size_t DD>
    static constexpr std::size_t dim(){
        return decay_traits<A>::template dim<DD>();
    }

    template<typename A, typename B, class Enable = void>
    struct result_type_builder {
        using type = dyn_matrix<value_t<A>, 2>;
    };

    template<typename A, typename B>
    struct result_type_builder<A, B, std::enable_if_t<all_fast<A,B>::value>> {
        using type = fast_dyn_matrix<value_t<A>, this_type::template dim<A, B, 0>(), this_type::template dim<A, B, 1>()>;
    };

    template<typename A, typename B>
    using result_type = typename result_type_builder<A, B>::type;

    template<typename A, typename B, cpp_enable_if(all_fast<A, B>::value)>
    static result_type<A,B>* allocate(A&& /*a*/, B&& /*b*/){
        return new result_type<A,B>();
    }

    template<typename A, typename B, cpp_disable_if(all_fast<A>::value)>
    static result_type<A,B>* allocate(A&& a, B&& /*b*/){
        return new result_type<A,B>(etl::dim<0>(a), etl::dim<1>(a));
    }

    template<typename A, typename B, typename C>
    static void apply(A&& a, B&& b, C&& c){
        static_assert(all_etl_expr<A, B, C>::value, "upsample_2d only supported for ETL expressions");
        static_assert(decay_traits<A>::dimensions() == 2 && decay_traits<A>::dimensions() == 2 && decay_traits<C>::dimensions() == 2, "upsample_2d needs 2D matrices");

        Impl<A, B, C>::template apply<C1, C2>(
            std::forward<A>(a),
            std::forward<B>(b),
            std::forward<C>(c));
    }

    static std::string desc() noexcept {
        return "upsample_2d";
    }

    template<typename A, typename B>
    static std::size_t dim(const A& a, const B& /*b*/, std::size_t d){
        return etl::dim(a, d);
    }

    template<typename A, typename B>
    static std::size_t size(const A& a, const B& /*b*/){
        return etl::dim<0>(a) * etl::dim<1>(a);
    }

    template<typename A, typename B>
    static constexpr std::size_t size(){
        return this_type::template dim<A, B, 0>() * this_type::template dim<A, B, 1>();
    }

    static constexpr std::size_t dimensions(){
        return 2;
    }
};

//Max Pool 2D

template<typename T, std::size_t C1, std::size_t C2>
using max_upsample_2d_expr = basic_upsample_2d_expr<T, C1, C2, impl::max_upsample_2d>;

template<typename T, std::size_t C1, std::size_t C2, std::size_t C3, template<typename...> class Impl>
struct basic_upsample_3d_expr {
    static_assert(C1 > 0, "C1 must be greater than 0");
    static_assert(C2 > 0, "C2 must be greater than 0");
    static_assert(C3 > 0, "C3 must be greater than 0");

    using this_type = basic_upsample_3d_expr<T, C1, C2, C3, Impl>;

    template<typename A, typename B, std::size_t DD>
    static constexpr std::size_t dim(){
        return decay_traits<A>::template dim<DD>();
    }

    template<typename A, typename B, class Enable = void>
    struct result_type_builder {
        using type = dyn_matrix<value_t<A>, 3>;
    };

    template<typename A, typename B>
    struct result_type_builder<A, B, std::enable_if_t<all_fast<A,B>::value>> {
        using type = fast_dyn_matrix<value_t<A>, this_type::template dim<A, B, 0>(), this_type::template dim<A, B, 1>(), this_type::template dim<A, B, 2>()>;
    };

    template<typename A, typename B>
    using result_type = typename result_type_builder<A, B>::type;

    template<typename A, typename B, cpp_enable_if(all_fast<A, B>::value)>
    static result_type<A,B>* allocate(A&& /*a*/, B&& /*b*/){
        return new result_type<A,B>();
    }

    template<typename A, typename B, cpp_disable_if(all_fast<A>::value)>
    static result_type<A,B>* allocate(A&& a, B&& /*b*/){
        return new result_type<A,B>(etl::dim<0>(a), etl::dim<1>(a), etl::dim<2>(a));
    }

    template<typename A, typename B, typename C>
    static void apply(A&& a, B&& b, C&& c){
        static_assert(all_etl_expr<A, B, C>::value, "upsample_2d only supported for ETL expressions");
        static_assert(decay_traits<A>::dimensions() == 3 && decay_traits<A>::dimensions() == 3 && decay_traits<C>::dimensions() == 3, "upsample_2d needs 2D matrices");

        Impl<A, B, C>::template apply<C1, C2, C3>(
            std::forward<A>(a),
            std::forward<B>(b),
            std::forward<C>(c));
    }

    static std::string desc() noexcept {
        return "upsample_2d";
    }

    template<typename A, typename B>
    static std::size_t dim(const A& a, const B& /*b*/, std::size_t d){
        return etl::dim(a, d);
    }

    template<typename A, typename B>
    static std::size_t size(const A& a, const B& /*b*/){
        return etl::dim<0>(a) * etl::dim<1>(a) * etl::dim<2>(a);
    }

    template<typename A, typename B>
    static constexpr std::size_t size(){
        return this_type::template dim<A, B, 0>() * this_type::template dim<A, B, 1>() * this_type::template dim<A, B, 2>();
    }

    static constexpr std::size_t dimensions(){
        return 3;
    }
};

//Max upsample 2D

template<typename T, std::size_t C1, std::size_t C2, std::size_t C3>
using max_upsample_3d_expr = basic_upsample_3d_expr<T, C1, C2, C3, impl::max_upsample_3d>;

} //end of namespace etl