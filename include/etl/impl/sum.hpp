//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*!
 * \file
 * \brief Selector for the "sum" reduction implementations.
 *
 * The functions are responsible for selecting the most efficient
 * implementation for each case, based on what is available. The selection of
 * parallel versus serial is also done at this level. The implementation
 * functions should never be used directly, only functions of this header can
 * be used directly.
 *
 * Note: In a perfect world (full constexpr function and variable templates),
 * the selection should be done with a template parameter in a variable
 * template full sspecialization (alias for each real functions).
 */

#pragma once

//Include the implementations
#include "etl/impl/std/sum.hpp"
#include "etl/impl/vec/sum.hpp"

namespace etl {

namespace detail {

/*!
 * \brief Select the sum implementation for an expression of type E
 *
 * This does not consider the local context
 *
 * \tparam E The type of expression
 * \return The implementation to use
 */
template <typename E>
cpp14_constexpr etl::sum_impl select_default_sum_impl() {
    //Note: since the constexpr values will be known at compile time, the
    //conditions will be a lot simplified

    if (vec_enabled && all_vectorizable<vector_mode, E>::value) {
        return etl::sum_impl::VEC;
    }

    return etl::sum_impl::STD;
}

/*!
 * \brief Select the sum implementation for an expression of type E
 * \tparam E The type of expression
 * \return The implementation to use
 */
template <typename E>
etl::sum_impl select_sum_impl() {
    if (local_context().sum_selector.forced) {
        auto forced = local_context().sum_selector.impl;

        switch (forced) {
            //VEC cannot always be used
            case sum_impl::VEC:
                if (!vec_enabled || !decay_traits<E>::template vectorizable<vector_mode>::value) {                                //COVERAGE_EXCLUDE_LINE
                    std::cerr << "Forced selection to VEC sum implementation, but not possible for this expression" << std::endl; //COVERAGE_EXCLUDE_LINE
                    return select_default_sum_impl<E>();                                                                          //COVERAGE_EXCLUDE_LINE
                }                                                                                                                 //COVERAGE_EXCLUDE_LINE

                return forced;

            //In other cases, simply use the forced impl
            default:
                return forced;
        }
    }

    return select_default_sum_impl<E>();
}

/*!
 * \brief Indicate if the sum must run in parallel for the given expression
 * \param e type The expression to sum
 * \return true if the implementation must run in parallel or not (false).
 */
template <typename E>
inline bool select_parallel(const E& e) {
    if ((is_parallel && !local_context().serial) || local_context().parallel) {
        return size(e) >= sum_parallel_threshold;
    } else {
        return false;
    }
}

/*!
 * \brief Sum operation implementation
 */
struct sum_impl {
    /*!
     * \brief Apply the functor to e
     */
    template <typename E>
    static value_t<E> apply(const E& e) {
        auto impl = select_sum_impl<E>();

        bool parallel_dispatch = select_parallel(e);

        value_t<E> acc(0);

        auto acc_functor = [&acc](value_t<E> value) {
            acc += value;
        };

        //TODO Make it so that dispatching aligns the sub parts

        if (impl == etl::sum_impl::VEC) {
            dispatch_1d_acc<value_t<E>>(parallel_dispatch, [&e](std::size_t first, std::size_t last) -> value_t<E> {
                return impl::vec::sum(e, first, last);
            }, acc_functor, 0, size(e));
        } else {
            dispatch_1d_acc<value_t<E>>(parallel_dispatch, [&e](std::size_t first, std::size_t last) -> value_t<E> {
                return impl::standard::sum(e, first, last);
            }, acc_functor, 0, size(e));
        }

        return acc;
    }
};

} //end of namespace detail

} //end of namespace etl
