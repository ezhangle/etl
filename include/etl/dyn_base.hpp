//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*!
 * \file
 * \brief Base class and utilities for dyn matrix implementations
 */

#pragma once

namespace etl {

/*!
 * \brief A simple type to use as init flag to constructor
 */
enum class init_flag_t {
    DUMMY  ///< Dummy value for the flag
};

/*!
 * \brief A simple value to use as init flag to constructor
 */
constexpr init_flag_t init_flag = init_flag_t::DUMMY;

/*!
 * \brief Simple collection of values to initialize a dyn matrix
 */
template <typename... V>
struct values_t {
    const std::tuple<V...> values; ///< The contained values

    /*!
     * \brief Construct a new sequence of values.
     */
    explicit values_t(V... v)
            : values(v...){};

    /*!
     * \brief Returns the sequence of values as a std::vector
     * \return a std::vector containing all the values
     */
    template <typename T>
    std::vector<T> list() const {
        return list_sub<T>(std::make_index_sequence<sizeof...(V)>());
    }

private:
    /*!
     * \brief Returns the sequence of values as a std::vector
     */
    template <typename T, std::size_t... I>
    std::vector<T> list_sub(const std::index_sequence<I...>& /*i*/) const {
        return {static_cast<T>(std::get<I>(values))...};
    }
};

/*!
 * \brief Create a list of values for initializing a dyn_matrix
 */
template <typename... V>
values_t<V...> values(V... v) {
    return values_t<V...>{v...};
}

namespace dyn_detail {

/*!
 * \brief Traits to test if the constructor is an init constructor
 */
template <typename... S>
struct is_init_constructor : std::false_type {};

/*!
 * \copydoc is_init_list_constructor
 */
template <typename S1, typename S2, typename S3, typename... S>
struct is_init_constructor<S1, S2, S3, S...> : std::is_same<init_flag_t, typename cpp::nth_type<sizeof...(S), S2, S3, S...>::type> {};

/*!
 * \brief Traits to test if the constructor is an initializer list constructor
 */
template <typename... S>
struct is_initializer_list_constructor : std::false_type {};

/*!
 * \copydoc is_initializer_list_constructor
 */
template <typename S1, typename S2, typename... S>
struct is_initializer_list_constructor<S1, S2, S...> : cpp::is_specialization_of<std::initializer_list, typename cpp::last_type<S2, S...>::type> {};

/*!
 * \brief Returns the size of a matrix given its dimensions
 */
inline std::size_t size(std::size_t first) {
    return first;
}

/*!
 * \brief Returns the size of a matrix given its dimensions
 */
template <typename... T>
inline std::size_t size(std::size_t first, T... args) {
    return first * size(args...);
}

/*!
 * \brief Returns the size of a matrix given its dimensions
 */
template <std::size_t... I, typename... T>
inline std::size_t size(const std::index_sequence<I...>& /*i*/, const T&... args) {
    return size((cpp::nth_value<I>(args...))...);
}

/*!
 * \brief Returns a collection of dimensions of the matrix.
 */
template <std::size_t... I, typename... T>
inline std::array<std::size_t, sizeof...(I)> sizes(const std::index_sequence<I...>& /*i*/, const T&... args) {
    return {{static_cast<std::size_t>(cpp::nth_value<I>(args...))...}};
}

} // end of namespace dyn_detail

/*!
 * \brief Matrix with run-time fixed dimensions.
 *
 * The matrix support an arbitrary number of dimensions.
 */
template <typename T, std::size_t D>
struct dyn_base {
    static_assert(D > 0, "A matrix must have a least 1 dimension");

protected:
    static constexpr std::size_t n_dimensions = D;                              ///< The number of dimensions
    static constexpr std::size_t alignment    = intrinsic_traits<T>::alignment; ///< The memory alignment

    using value_type             = T;                                     ///< The value type
    using dimension_storage_impl = std::array<std::size_t, n_dimensions>; ///< The type used to store the dimensions
    using memory_type            = value_type*;                           ///< The memory type
    using const_memory_type      = const value_type*;                     ///< The const memory type

    std::size_t _size;                  ///< The size of the matrix
    dimension_storage_impl _dimensions; ///< The dimensions of the matrix

    /*!
     * \brief Verify some invariants with assertions
     *
     * This function should only be used internally to ensure that
     * no breaking changes are made.
     */
    void check_invariants() {
        cpp_assert(_dimensions.size() == D, "Invalid dimensions");

#ifndef NDEBUG
        auto computed = std::accumulate(_dimensions.begin(), _dimensions.end(), std::size_t(1), std::multiplies<std::size_t>());
        cpp_assert(computed == _size, "Incoherency in dimensions");
#endif
    }

    /*!
     * \brief Allocate aligned memory for n elements of the given type
     * \tparam M the type of objects to allocate
     * \param n The number of elements to allocate
     * \return The allocated memory
     */
    template <typename M = value_type>
    static M* allocate(std::size_t n) {
        M* memory = aligned_allocator<alignment>::template allocate<M>(n);
        cpp_assert(memory, "Impossible to allocate memory for dyn_matrix");
        cpp_assert(reinterpret_cast<uintptr_t>(memory) % alignment == 0, "Failed to align memory of matrix");

        //In case of non-trivial type, we need to call the constructors
        if (!std::is_trivial<M>::value) {
            new (memory) M[n]();
        }

        if(padding){
            std::fill_n(memory, n, M());
        }

        return memory;
    }

    /*!
     * \brief Release aligned memory for n elements of the given type
     * \param ptr Pointer to the memory to release
     * \param n The number of elements to release
     */
    template <typename M>
    static void release(M* ptr, std::size_t n) {
        //In case of non-trivial type, we need to call the destructors
        if (!std::is_trivial<M>::value) {
            for (std::size_t i = 0; i < n; ++i) {
                ptr[i].~M();
            }
        }

        aligned_allocator<alignment>::template release<M>(ptr);
    }

    /*!
     * \brief Initialize the dyn_base with a size of 0
     */
    dyn_base() noexcept : _size(0) {
        std::fill(_dimensions.begin(), _dimensions.end(), 0);

        check_invariants();
    }

    /*!
     * \brief Copy construct a dyn_base
     * \param rhs The dyn_base to copy from
     */
    dyn_base(const dyn_base& rhs) noexcept = default;

    /*!
     * \brief Move construct a dyn_base
     * \param rhs The dyn_base to move from
     */
    dyn_base(dyn_base&& rhs) noexcept = default;

    /*!
     * \brief Construct a dyn_base if the given size and dimensions
     * \param size The size of the matrix
     * \param dimensions The dimensions of the matrix
     */
    dyn_base(std::size_t size, dimension_storage_impl dimensions) noexcept : _size(size), _dimensions(dimensions) {
        check_invariants();
    }

    /*!
     * \brief Move construct a dyn_base
     * \param rhs The dyn_base to move from
     */
    template <typename E>
    dyn_base(E&& rhs)
            : _size(etl::size(rhs)) {
        for (std::size_t d = 0; d < etl::dimensions(rhs); ++d) {
            _dimensions[d] = etl::dim(rhs, d);
        }

        check_invariants();
    }

public:
    /*!
     * \brief Returns the number of dimensions of the matrix
     * \return the number of dimensions of the matrix
     */
    static constexpr std::size_t dimensions() noexcept {
        return n_dimensions;
    }

    /*!
     * \brief Returns the size of the matrix, in O(1)
     * \return The size of the matrix
     */
    std::size_t size() const noexcept {
        return _size;
    }

    /*!
     * \brief Returns the number of rows of the matrix (the first dimension)
     * \return The number of rows of the matrix
     */
    std::size_t rows() const noexcept {
        return _dimensions[0];
    }

    /*!
     * \brief Returns the number of columns of the matrix (the first dimension)
     * \return The number of columns of the matrix
     */
    std::size_t columns() const noexcept {
        static_assert(n_dimensions > 1, "columns() only valid for 2D+ matrices");
        return _dimensions[1];
    }

    /*!
     * \brief Returns the dth dimension of the matrix
     * \param d The dimension to get
     * \return The Dth dimension of the matrix
     */
    std::size_t dim(std::size_t d) const noexcept {
        cpp_assert(d < n_dimensions, "Invalid dimension");

        return _dimensions[d];
    }

    /*!
     * \brief Returns the D2th dimension of the matrix
     * \return The D2th dimension of the matrix
     */
    template <std::size_t D2>
    std::size_t dim() const noexcept {
        cpp_assert(D2 < n_dimensions, "Invalid dimension");

        return _dimensions[D2];
    }
};

/*!
 *
 * \brief Dense Matrix with run-time fixed dimensions.
 * The matrix support an arbitrary number of dimensions.
 */
template <typename Derived, typename T, order SO, std::size_t D>
struct dense_dyn_base : dyn_base<T, D> {
    using value_type        = T;
    using base_type         = dyn_base<T, D>;
    using derived_t         = Derived;
    using memory_type       = value_type*;       ///< The memory type
    using const_memory_type = const value_type*; ///< The const memory type
    using iterator          = memory_type;       ///< The type of iterator
    using const_iterator    = const_memory_type; ///< The type of const iterator

    using dimension_storage_impl = typename base_type::dimension_storage_impl;

    static constexpr std::size_t n_dimensions = D;  ///< The number of dimensions
    static constexpr order storage_order      = SO; ///< The storage order

    using base_type::_size;
    using base_type::dim;

    value_type* _memory = nullptr; ///< Pointer to the allocated memory

    /*!
     * \brief Initialize the dense_dyn_base with a size of 0
     */
    dense_dyn_base() noexcept : base_type() {
        //Nothing else to init
    }

    /*!
     * \brief Copy construct a dense_dyn_base
     * \param rhs The dense_dyn_base to copy from
     */
    dense_dyn_base(const dense_dyn_base& rhs) noexcept : base_type(rhs) {
        //Nothing else to init
    }

    /*!
     * \brief Move construct a dense_dyn_base
     * \param rhs The dense_dyn_base to move from
     */
    dense_dyn_base(dense_dyn_base&& rhs) noexcept : base_type(std::move(rhs)) {
        //Nothing else to init
    }

    /*!
     * \brief Construct a dense_dyn_base if the given size and dimensions
     * \param size The size of the matrix
     * \param dimensions The dimensions of the matrix
     */
    dense_dyn_base(std::size_t size, dimension_storage_impl dimensions) noexcept : base_type(size, dimensions) {
        //Nothing else to init
    }

    /*!
     * \brief Move construct a dense_dyn_base
     * \param rhs The dense_dyn_base to move from
     */
    template <typename E>
    explicit dense_dyn_base(E&& rhs) : base_type(std::move(rhs)) {
        //Nothing else to init
    }

    /*!
     * \brief Return the flat index for the element at the given position
     * \param sizes The indices
     * \return The flat index
     */
    template <typename... S>
    std::size_t index(S... sizes) const noexcept(assert_nothrow) {
        //Note: Version with sizes moved to a std::array and accessed with
        //standard loop may be faster, but need some stack space (relevant ?)

        std::size_t index = 0;

        if (storage_order == order::RowMajor) {
            std::size_t subsize = _size;
            std::size_t i       = 0;

            cpp::for_each_in(
                [&subsize, &index, &i, this](std::size_t s) {
                    cpp_assert(s < this->dim(i), "Out of bounds");
                    subsize /= this->dim(i++);
                    index += subsize * s;
                },
                sizes...);
        } else {
            std::size_t subsize = 1;
            std::size_t i       = 0;

            cpp::for_each_in(
                [&subsize, &index, &i, this](std::size_t s) {
                    cpp_assert(s < this->dim(i), "Out of bounds");
                    index += subsize * s;
                    subsize *= this->dim(i++);
                },
                sizes...);
        }

        return index;
    }

    /*!
     * \brief Access the ith element of the matrix
     * \param i The index of the element to search
     * \return a reference to the ith element
     *
     * Accessing an element outside the matrix results in Undefined Behaviour.
     */
    template <bool B = n_dimensions == 1, cpp_enable_if(B)>
    value_type& operator()(std::size_t i) noexcept {
        cpp_assert(i < dim(0), "Out of bounds");

        return _memory[i];
    }

    /*!
     * \brief Access the ith element of the matrix
     * \param i The index of the element to search
     * \return a reference to the ith element
     *
     * Accessing an element outside the matrix results in Undefined Behaviour.
     */
    template <bool B = n_dimensions == 1, cpp_enable_if(B)>
    const value_type& operator()(std::size_t i) const noexcept {
        cpp_assert(i < dim(0), "Out of bounds");

        return _memory[i];
    }

    /*!
     * \brief Access the (i, j) element of the 2D matrix
     * \param i The index of the first dimension
     * \param j The index of the second dimension
     * \return a reference to the (i,j) element
     *
     * Accessing an element outside the matrix results in Undefined Behaviour.
     */
    template <bool B = n_dimensions == 2, cpp_enable_if(B)>
    value_type& operator()(std::size_t i, std::size_t j) noexcept {
        cpp_assert(i < dim(0), "Out of bounds");
        cpp_assert(j < dim(1), "Out of bounds");

        if (storage_order == order::RowMajor) {
            return _memory[i * dim(1) + j];
        } else {
            return _memory[j * dim(0) + i];
        }
    }

    /*!
     * \brief Access the (i, j) element of the 2D matrix
     * \param i The index of the first dimension
     * \param j The index of the second dimension
     * \return a reference to the (i,j) element
     *
     * Accessing an element outside the matrix results in Undefined Behaviour.
     */
    template <bool B = n_dimensions == 2, cpp_enable_if(B)>
    const value_type& operator()(std::size_t i, std::size_t j) const noexcept {
        cpp_assert(i < dim(0), "Out of bounds");
        cpp_assert(j < dim(1), "Out of bounds");

        if (storage_order == order::RowMajor) {
            return _memory[i * dim(1) + j];
        } else {
            return _memory[j * dim(0) + i];
        }
    }

    /*!
     * \brief Returns the value at the position (sizes...)
     * \param sizes The indices
     * \return The value at the position (sizes...)
     */
    template <typename... S, cpp_enable_if(
                                 (n_dimensions > 2),
                                 (sizeof...(S) == n_dimensions),
                                 cpp::all_convertible_to<std::size_t, S...>::value)>
    const value_type& operator()(S... sizes) const noexcept(assert_nothrow) {
        static_assert(sizeof...(S) == n_dimensions, "Invalid number of parameters");

        return _memory[index(sizes...)];
    }

    /*!
     * \brief Returns the value at the position (sizes...)
     * \param sizes The indices
     * \return The value at the position (sizes...)
     */
    template <typename... S, cpp_enable_if(
                                 (n_dimensions > 2),
                                 (sizeof...(S) == n_dimensions),
                                 cpp::all_convertible_to<std::size_t, S...>::value)>
    value_type& operator()(S... sizes) noexcept(assert_nothrow) {
        static_assert(sizeof...(S) == n_dimensions, "Invalid number of parameters");

        return _memory[index(sizes...)];
    }

    /*!
     * \brief Returns the element at the given index
     * \param i The index
     * \return a reference to the element at the given index.
     */
    const value_type& operator[](std::size_t i) const noexcept {
        cpp_assert(i < _size, "Out of bounds");

        return _memory[i];
    }

    /*!
     * \brief Returns the element at the given index
     * \param i The index
     * \return a reference to the element at the given index.
     */
    value_type& operator[](std::size_t i) noexcept {
        cpp_assert(i < _size, "Out of bounds");

        return _memory[i];
    }

    /*!
     * \returns the value at the given index
     * This function never alters the state of the container.
     * \param i The index
     * \return the value at the given index.
     */
    value_type read_flat(std::size_t i) const noexcept {
        cpp_assert(i < _size, "Out of bounds");

        return _memory[i];
    }

    /*!
     * \brief Test if this expression aliases with the given expression
     * \param rhs The other expression to test
     * \return true if the two expressions aliases, false otherwise
     */
    template<typename E, cpp_enable_if(all_dma<E>::value)>
    bool alias(const E& rhs) const noexcept {
        return memory_alias(memory_start(), memory_end(), rhs.memory_start(), rhs.memory_end());
    }

    /*!
     * \brief Test if this expression aliases with the given expression
     * \param rhs The other expression to test
     * \return true if the two expressions aliases, false otherwise
     */
    template<typename E, cpp_disable_if(all_dma<E>::value)>
    bool alias(const E& rhs) const noexcept {
        return rhs.alias(as_derived());
    }

    /*!
     * \brief Return an iterator to the first element of the matrix
     * \return an iterator pointing to the first element of the matrix
     */
    iterator begin() noexcept {
        return _memory;
    }

    /*!
     * \brief Return an iterator to the past-the-end element of the matrix
     * \return an iterator pointing to the past-the-end element of the matrix
     */
    iterator end() noexcept {
        return _memory + _size;
    }

    /*!
     * \brief Return an iterator to the first element of the matrix
     * \return an iterator pointing to the first element of the matrix
     */
    const_iterator begin() const noexcept {
        return _memory;
    }

    /*!
     * \brief Return an iterator to the past-the-end element of the matrix
     * \return an iterator pointing to the past-the-end element of the matrix
     */
    const_iterator end() const noexcept {
        return _memory + _size;
    }

    /*!
     * \brief Return a const iterator to the first element of the matrix
     * \return an const iterator pointing to the first element of the matrix
     */
    const_iterator cbegin() const noexcept {
        return _memory;
    }

    /*!
     * \brief Return a const iterator to the past-the-end element of the matrix
     * \return a const iterator pointing to the past-the-end element of the matrix
     */
    const_iterator cend() const noexcept {
        return _memory + _size;
    }

    /*!
     * \brief Returns a pointer to the first element in memory.
     * \return a pointer tot the first element in memory.
     */
    inline memory_type memory_start() noexcept {
        return _memory;
    }

    /*!
     * \brief Returns a pointer to the first element in memory.
     * \return a pointer tot the first element in memory.
     */
    inline const_memory_type memory_start() const noexcept {
        return _memory;
    }

    /*!
     * \brief Returns a pointer to the past-the-end element in memory.
     * \return a pointer tot the past-the-end element in memory.
     */
    memory_type memory_end() noexcept {
        return _memory + _size;
    }

    /*!
     * \brief Returns a pointer to the past-the-end element in memory.
     * \return a pointer to the past-the-end element in memory.
     */
    const_memory_type memory_end() const noexcept {
        return _memory + _size;
    }


    /*!
     * \brief Multiply each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    derived_t& operator+=(const value_type& rhs) noexcept {
        detail::scalar_add::apply(as_derived(), rhs);
        return as_derived();
    }

    /*!
     * \brief Multiply each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    derived_t& operator+=(const R& rhs) noexcept {
        validate_expression(as_derived(), rhs);
        add_evaluate(rhs, as_derived());
        return as_derived();
    }

    /*!
     * \brief Multiply each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    derived_t& operator-=(const value_type& rhs) noexcept {
        detail::scalar_sub::apply(as_derived(), rhs);
        return as_derived();
    }

    /*!
     * \brief Multiply each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    derived_t& operator-=(const R& rhs) noexcept {
        validate_expression(as_derived(), rhs);
        sub_evaluate(rhs, as_derived());
        return as_derived();
    }

    /*!
     * \brief Multiply each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    derived_t& operator*=(const value_type& rhs) noexcept {
        detail::scalar_mul::apply(as_derived(), rhs);
        return as_derived();
    }

    /*!
     * \brief Multiply each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    derived_t& operator*=(const R& rhs) noexcept {
        validate_expression(as_derived(), rhs);
        mul_evaluate(rhs, as_derived());
        return as_derived();
    }

    /*!
     * \brief Multiply each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    derived_t& operator>>=(const value_type& rhs) noexcept {
        detail::scalar_mul::apply(as_derived(), rhs);
        return as_derived();
    }

    /*!
     * \brief Multiply each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    derived_t& operator>>=(const R& rhs) noexcept {
        validate_expression(as_derived(), rhs);
        mul_evaluate(rhs, as_derived());
        return as_derived();
    }

    /*!
     * \brief Divide each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    derived_t& operator/=(const value_type& rhs) noexcept {
        detail::scalar_div::apply(as_derived(), rhs);
        return as_derived();
    }

    /*!
     * \brief Modulo each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    derived_t& operator/=(const R& rhs) noexcept {
        validate_expression(as_derived(), rhs);
        div_evaluate(rhs, as_derived());
        return as_derived();
    }

    /*!
     * \brief Modulo each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    derived_t& operator%=(const value_type& rhs) noexcept {
        detail::scalar_mod::apply(as_derived(), rhs);
        return as_derived();
    }

    /*!
     * \brief Modulo each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    derived_t& operator%=(const R& rhs) noexcept {
        validate_expression(as_derived(), rhs);
        mod_evaluate(rhs, as_derived());
        return as_derived();
    }

    /*!
     * \brief Creates a sub view of the matrix, effectively removing the first dimension and fixing it to the given index.
     * \param i The index to use
     * \return a sub view of the matrix at position i.
     */
    template <bool B = (n_dimensions > 1), cpp_enable_if(B)>
    auto operator()(std::size_t i) noexcept {
        return sub(as_derived(), i);
    }

    /*!
     * \brief Creates a sub view of the matrix, effectively removing the first dimension and fixing it to the given index.
     * \param i The index to use
     * \return a sub view of the matrix at position i.
     */
    template <bool B = (n_dimensions > 1), cpp_enable_if(B)>
    auto operator()(std::size_t i) const noexcept {
        return sub(as_derived(), i);
    }

    /*!
     * \brief Creates a slice view of the matrix, effectively reducing the first dimension.
     * \param first The first index to use
     * \param last The last index to use
     * \return a slice view of the matrix at position i.
     */
    auto slice(std::size_t first, std::size_t last) noexcept {
        return etl::slice(as_derived(), first, last);
    }

    /*!
     * \brief Creates a slice view of the matrix, effectively reducing the first dimension.
     * \param first The first index to use
     * \param last The last index to use
     * \return a slice view of the matrix at position i.
     */
    auto slice(std::size_t first, std::size_t last) const noexcept {
        return etl::slice(as_derived(), first, last);
    }
private:
    /*!
     * \brief Returns a reference to the derived object, i.e. the object using the CRTP injector.
     * \return a reference to the derived object.
     */
    derived_t& as_derived() noexcept {
        return *static_cast<derived_t*>(this);
    }

    /*!
     * \brief Returns a reference to the derived object, i.e. the object using the CRTP injector.
     * \return a reference to the derived object.
     */
    const derived_t& as_derived() const noexcept {
        return *static_cast<const derived_t*>(this);
    }
};

} //end of namespace etl
