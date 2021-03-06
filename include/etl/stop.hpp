//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

namespace etl {

/*!
 * \brief Force the evaluation of the given expression
 * \param value The ETL expression
 * \return A value class with the values of the given expression
 */
template <typename T, cpp_enable_if(is_etl_expr<T>::value, !etl_traits<T>::is_value, !etl_traits<T>::is_fast)>
auto s(T&& value) {
    // Sizes will be directly propagated
    return dyn_matrix<typename T::value_type, etl_traits<T>::dimensions()>(std::forward<T>(value));
}

/*!
 * \brief TMP struct to build fast matrix type from a fast expression type
 */
template <typename M, typename Sequence>
struct build_matrix_type;

/*!
 * \copydoc build_matrix_type
 */
template <typename M, std::size_t... I>
struct build_matrix_type<M, std::index_sequence<I...>> {
    using type = fast_matrix<typename M::value_type, etl_traits<M>::template dim<I>()...>; ///< The fast matrix type
};

/*!
 * \brief Force the evaluation of the given expression
 * \param value The ETL expression
 * \return A value class with the values of the given expression
 */
template <typename T, cpp_enable_if(is_etl_expr<T>::value, !etl_traits<T>::is_value, etl_traits<T>::is_fast)>
auto s(T&& value) {
    return typename build_matrix_type<T, std::make_index_sequence<etl_traits<T>::dimensions()>>::type(std::forward<T>(value));
}

} // end of namespace etl
