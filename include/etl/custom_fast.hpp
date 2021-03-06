//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*!
 * \file
 * \brief Contains custom static matrix implementation
 */

#pragma once

#include "etl/fast_base.hpp"

namespace etl {

/*!
 * \brief Matrix with compile-time fixed dimensions.
 *
 * The matrix support an arbitrary number of dimensions.
 */
template <typename T, typename ST, order SO, std::size_t... Dims>
struct custom_fast_matrix_impl final :
        fast_matrix_base<custom_fast_matrix_impl<T, ST, SO, Dims...>, T, ST, SO, Dims...>,
        inplace_assignable<custom_fast_matrix_impl<T, ST, SO, Dims...>>,
        comparable<custom_fast_matrix_impl<T, ST, SO, Dims...>>,
        expression_able<custom_fast_matrix_impl<T, ST, SO, Dims...>>,
        value_testable<custom_fast_matrix_impl<T, ST, SO, Dims...>>,
        dim_testable<custom_fast_matrix_impl<T, ST, SO, Dims...>> {
    static_assert(sizeof...(Dims) > 0, "At least one dimension must be specified");

public:
    static constexpr std::size_t n_dimensions = sizeof...(Dims);                      ///< The number of dimensions
    static constexpr std::size_t etl_size     = mul_all<Dims...>::value;              ///< The size of the matrix
    static constexpr order storage_order      = SO;                                   ///< The storage order
    static constexpr bool array_impl          = !matrix_detail::is_vector<ST>::value; ///< true if the storage is an std::arraw, false otherwise

    using this_type         = custom_fast_matrix_impl<T, ST, SO, Dims...>;     ///< this type
    using base_type         = fast_matrix_base<this_type, T, ST, SO, Dims...>; ///< The base type
    using value_type        = T;                                               ///< The value type
    using storage_impl      = ST;                                              ///< The storage implementation
    using memory_type       = value_type*;                                     ///< The memory type
    using const_memory_type = const value_type*;                               ///< The const memory type

    using base_type::dim;
    using base_type::size;
    using base_type::begin;
    using base_type::end;
    using base_type::memory_start;
    using base_type::memory_end;

    /*!
     * \brief The vectorization type for V
     */
    template <typename V = default_vec>
    using vec_type       = typename V::template vec_type<T>;

private:
    using base_type::_data;

    mutable gpu_handler<T> _gpu_memory_handler; ///< The GPU memory handler

public:
    /// Construction

    /*!
     * \brief Construct a fast matrix directly from storage memory
     * \param memory The unmanaged memory
     */
    explicit custom_fast_matrix_impl(memory_type memory) : base_type(storage_impl{memory, etl_size}) {
        //Nothing else to init
    }

    /*!
     * \brief Copy construct a fast matrix
     * \param rhs The fast matrix to copy from
     */
    custom_fast_matrix_impl(const custom_fast_matrix_impl& rhs) noexcept : base_type(rhs) {
        // Nothing else to init
    }

    /*!
     * \brief Move construct a fast matrix
     * \param rhs The fast matrix to move from
     */
    custom_fast_matrix_impl(custom_fast_matrix_impl&& rhs) noexcept : base_type(std::move(rhs)), _gpu_memory_handler(std::move(rhs._gpu_memory_handler)) {
        // Nothing else to init
    }

    // Assignment

    /*!
     * \brief Copy assign a fast matrix
     * \param rhs The fast matrix to copy from
     * \return a reference to the fast matrix
     */
    custom_fast_matrix_impl& operator=(const custom_fast_matrix_impl& rhs) noexcept {
        if (this != &rhs) {
            direct_copy(rhs.memory_start(), rhs.memory_end(), memory_start());
        }

        return *this;
    }

    /*!
     * \brief Copy assign a fast matrix from a different matrix fast matrix type
     * \param rhs The fast matrix to copy from
     * \return a reference to the fast matrix
     */
    template <std::size_t... SDims>
    custom_fast_matrix_impl& operator=(const custom_fast_matrix_impl<T, ST, SO, SDims...>& rhs) noexcept {
        validate_assign(*this, rhs);
        assign_evaluate(rhs, *this);
        return *this;
    }

    /*!
     * \brief Move assign a fast matrix
     * \param rhs The fast matrix to move from
     * \return a reference to the fast matrix
     */
    custom_fast_matrix_impl& operator=(custom_fast_matrix_impl&& rhs) noexcept {
        if (this != &rhs) {
            _data               = std::move(rhs._data);
            _gpu_memory_handler = std::move(rhs._gpu_memory_handler);
        }

        return *this;
    }

    /*!
     * \brief Assign the values of the STL container to the fast matrix
     * \param container The STL container to get the values from
     * \return a reference to the fast matrix
     */
    template <typename C, cpp_enable_if(!std::is_same<C, value_type>::value, std::is_convertible<value_t<C>, value_type>::value)>
    custom_fast_matrix_impl& operator=(const C& container) noexcept {
        validate_assign(*this, container);
        std::copy(container.begin(), container.end(), begin());
        return *this;
    }

    /*!
     * \brief Assign the values of the ETL expression to the fast matrix
     * \param e The ETL expression to get the values from
     * \return a reference to the fast matrix
     */
    template <typename E, cpp_enable_if(std::is_convertible<typename E::value_type, value_type>::value, is_etl_expr<E>::value)>
    custom_fast_matrix_impl& operator=(E&& e) {
        validate_assign(*this, e);
        assign_evaluate(std::forward<E>(e), *this);
        return *this;
    }

    /*!
     * \brief Assign the value to each element
     * \param value The value to assign to each element
     * \return a reference to the fast matrix
     */
    template <typename VT, cpp_enable_if_or(std::is_convertible<VT, value_type>::value, std::is_assignable<T&, VT>::value)>
    custom_fast_matrix_impl& operator=(const VT& value) noexcept {
        std::fill(_data.begin(), _data.end(), value);

        return *this;
    }

    // Swap operations

    /*!
     * \brief Swap the contents of the matrix with another matrix
     * \param other The other matrix
     */
    void swap(custom_fast_matrix_impl& other) {
        using std::swap;
        swap(_data, other._data);
    }

    // Accessors

    /*!
     * \brief Store several elements in the matrix at once
     * \param in The several elements to store
     * \param i The position at which to start. This will be aligned from the beginning (multiple of the vector size).
     * \tparam V The vectorization mode to use
     */
    template <typename V = default_vec>
    void store(vec_type<V> in, std::size_t i) noexcept {
        V::storeu(memory_start() + i, in);
    }

    /*!
     * \brief Store several elements in the matrix at once
     * \param in The several elements to store
     * \param i The position at which to start. This will be aligned from the beginning (multiple of the vector size).
     * \tparam V The vectorization mode to use
     */
    template <typename V = default_vec>
    void storeu(vec_type<V> in, std::size_t i) noexcept {
        V::storeu(memory_start() + i, in);
    }

    /*!
     * \brief Store several elements in the matrix at once, using non-temporal store
     * \param in The several elements to store
     * \param i The position at which to start. This will be aligned from the beginning (multiple of the vector size).
     * \tparam V The vectorization mode to use
     */
    template <typename V = default_vec>
    void stream(vec_type<V> in, std::size_t i) noexcept {
        V::storeu(memory_start() + i, in);
    }

    /*!
     * \brief Load several elements of the matrix at once
     * \param i The position at which to start. This will be aligned from the beginning (multiple of the vector size).
     * \tparam V The vectorization mode to use
     * \return a vector containing several elements of the matrix
     */
    template <typename V = default_vec>
    vec_type<V> load(std::size_t i) const noexcept {
        return V::loadu(memory_start() + i);
    }

    /*!
     * \brief Load several elements of the matrix at once
     * \param i The position at which to start. This will be aligned from the beginning (multiple of the vector size).
     * \tparam V The vectorization mode to use
     * \return a vector containing several elements of the matrix
     */
    template <typename V = default_vec>
    vec_type<V> loadu(std::size_t i) const noexcept {
        return V::loadu(memory_start() + i);
    }

    /*!
     * \brief Return an opaque (type-erased) access to the memory of the matrix
     * \return a structure containing the dimensions, the storage order and the memory pointers of the matrix
     */
    opaque_memory<T, n_dimensions> direct() const {
        return opaque_memory<T, n_dimensions>(memory_start(), etl_size, {{Dims...}}, _gpu_memory_handler, SO);
    }
};

static_assert(std::is_nothrow_default_constructible<fast_vector<double, 2>>::value, "fast_vector should be nothrow default constructible");
static_assert(std::is_nothrow_copy_constructible<fast_vector<double, 2>>::value, "fast_vector should be nothrow copy constructible");
static_assert(std::is_nothrow_move_constructible<fast_vector<double, 2>>::value, "fast_vector should be nothrow move constructible");
static_assert(std::is_nothrow_copy_assignable<fast_vector<double, 2>>::value, "fast_vector should be nothrow copy assignable");
static_assert(std::is_nothrow_move_assignable<fast_vector<double, 2>>::value, "fast_vector should be nothrow move assignable");
static_assert(std::is_nothrow_destructible<fast_vector<double, 2>>::value, "fast_vector should be nothrow destructible");

/*!
 * \brief Swaps the given two matrices
 * \param lhs The first matrix to swap
 * \param rhs The second matrix to swap
 */
template <typename T, typename ST, order SO, std::size_t... Dims>
void swap(custom_fast_matrix_impl<T, ST, SO, Dims...>& lhs, custom_fast_matrix_impl<T, ST, SO, Dims...>& rhs) {
    lhs.swap(rhs);
}

/*!
 * \brief Prints a fast matrix type (not the contents) to the given stream
 * \param os The output stream
 * \param matrix The fast matrix to print
 * \return the output stream
 */
template <typename T, typename ST, order SO, std::size_t... Dims>
std::ostream& operator<<(std::ostream& os, const custom_fast_matrix_impl<T, ST, SO, Dims...>& matrix) {
    cpp_unused(matrix);

    if (sizeof...(Dims) == 1) {
        return os << "V[" << concat_sizes(Dims...) << "]";
    }

    return os << "M[" << concat_sizes(Dims...) << "]";
}

} //end of namespace etl
