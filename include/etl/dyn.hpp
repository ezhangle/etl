//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_DYN_MATRIX_IMPL_HPP
#define ETL_DYN_MATRIX_IMPL_HPP

#include <vector>       //To store the data
#include <array>        //To store the dimensions
#include <tuple>        //For TMP stuff
#include <algorithm>    //For std::find_if
#include <iostream>     //For stream support

#include "cpp_utils/assert.hpp"
#include "cpp_utils/tmp.hpp"

#include "evaluator.hpp"
#include "traits_lite.hpp"          //forward declaration of the traits
#include "compat.hpp"               //To make it work with g++

// CRTP classes
#include "crtp/inplace_assignable.hpp"
#include "crtp/comparable.hpp"
#include "crtp/iterable.hpp"
#include "crtp/expression_able.hpp"

namespace etl {

enum class init_flag_t { DUMMY };
constexpr const init_flag_t init_flag = init_flag_t::DUMMY;

template<typename... V>
struct values_t {
    const std::tuple<V...> values;
    values_t(V... v) : values(v...) {};

    template<typename T, std::size_t... I>
    std::vector<T> list_sub(const std::index_sequence<I...>& /*i*/) const {
        return {static_cast<T>(std::get<I>(values))...};
    }

    template<typename T>
    std::vector<T> list() const {
        return list_sub<T>(std::make_index_sequence<sizeof...(V)>());
    }
};

template<typename... V>
values_t<V...> values(V... v){
    return {v...};
}

namespace dyn_detail {

template<typename... S>
struct is_init_constructor : std::false_type {};

template<typename S1, typename S2, typename S3, typename... S>
struct is_init_constructor<S1, S2, S3, S...> :
    std::is_same<init_flag_t, typename cpp::nth_type<1+sizeof...(S), S1, S2, S3, S...>::type> {};

template<typename... S>
struct is_initializer_list_constructor : std::false_type {};

template<typename S1, typename S2, typename... S>
struct is_initializer_list_constructor<S1, S2, S...> :
    cpp::is_specialization_of<std::initializer_list, typename cpp::last_type<S1, S2, S...>::type> {};

inline std::size_t size(std::size_t first){
    return first;
}

template<typename... T>
inline std::size_t size(std::size_t first, T... args){
    return first * size(args...);
}

template<std::size_t... I, typename... T>
inline std::size_t size(const std::index_sequence<I...>& /*i*/, const T&... args){
    return size((cpp::nth_value<I>(args...))...);
}

template<std::size_t... I, typename... T>
inline std::array<std::size_t, sizeof...(I)> sizes(const std::index_sequence<I...>& /*i*/, const T&... args){
    return {{static_cast<std::size_t>(cpp::nth_value<I>(args...))...}};
}

} // end of namespace dyn_detail

template<typename T, order SO, std::size_t D>
struct dyn_matrix_impl final :
        inplace_assignable<dyn_matrix_impl<T, SO, D>>,
        comparable<dyn_matrix_impl<T, SO, D>>,
        expression_able<dyn_matrix_impl<T, SO, D>>,
        iterable<dyn_matrix_impl<T, SO, D>> {
    static_assert(D > 0, "A matrix must have a least 1 dimension");

public:
    static constexpr const std::size_t n_dimensions = D;
    static constexpr const order storage_order = SO;

    using                value_type = T;
    using              storage_impl = std::vector<value_type>;
    using    dimension_storage_impl = std::array<std::size_t, n_dimensions>;
    using                  iterator = typename storage_impl::iterator;
    using            const_iterator = typename storage_impl::const_iterator;
    using               memory_type = value_type*;
    using         const_memory_type = const value_type*;
    using                  vec_type = intrinsic_type<T>;

private:
    const std::size_t _size;
    storage_impl _data;
    dimension_storage_impl _dimensions;

public:
    //{{{ Construction

    //Default copy constructor
    dyn_matrix_impl(const dyn_matrix_impl& rhs) = default;

    //Initializer-list construction for vector
    dyn_matrix_impl(std::initializer_list<value_type> list) :
            _size(list.size()),
            _data(list),
            _dimensions{{list.size()}} {
        static_assert(n_dimensions == 1, "This constructor can only be used for 1D matrix");
        //Nothing to init
    }

    //Normal constructor with only sizes
    template<typename... S, cpp_enable_if(
            (sizeof...(S) == D),
            cpp::all_convertible_to<std::size_t, S...>::value,
            cpp::is_homogeneous<typename cpp::first_type<S...>::type, S...>::value
        )>
    dyn_matrix_impl(S... sizes) :
            _size(dyn_detail::size(sizes...)),
            _data(_size),
            _dimensions{{static_cast<std::size_t>(sizes)...}} {
        //Nothing to init
    }

    //Sizes followed by an initializer list
    template<typename... S, cpp_enable_if(dyn_detail::is_initializer_list_constructor<S...>::value)>
    dyn_matrix_impl(S... sizes) :
            _size(dyn_detail::size(std::make_index_sequence<(sizeof...(S)-1)>(), sizes...)),
            _data(cpp::last_value(sizes...)),
            _dimensions(dyn_detail::sizes(std::make_index_sequence<(sizeof...(S)-1)>(), sizes...)) {
        static_assert(sizeof...(S) == D + 1, "Invalid number of dimensions");
    }

    //Sizes followed by a values_t
    template<typename S1, typename... S, cpp_enable_if(
            (sizeof...(S) == D),
            cpp::is_specialization_of<values_t, typename cpp::last_type<S1, S...>::type>::value
        )>
    dyn_matrix_impl(S1 s1, S... sizes) :
            _size(dyn_detail::size(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...)),
            _data(cpp::last_value(s1, sizes...).template list<value_type>()),
            _dimensions(dyn_detail::sizes(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...)) {
        //Nothing to init
    }

    //Sizes followed by a value
    template<typename S1, typename... S, cpp_enable_if(
            (sizeof...(S) == D),
            std::is_convertible<std::size_t, typename cpp::first_type<S1, S...>::type>::value,   //The first type must be convertible to size_t
            cpp::is_sub_homogeneous<S1, S...>::value,                                            //The first N-1 types must homegeneous
            (std::is_arithmetic<typename cpp::last_type<S1, S...>::type>::value
                ? std::is_convertible<value_type, typename cpp::last_type<S1, S...>::type>::value      //The last type must be convertible to value_type
                : std::is_same<value_type, typename cpp::last_type<S1, S...>::type>::value             //The last type must be exactly value_type
            )
        )>
    dyn_matrix_impl(S1 s1, S... sizes) :
            _size(dyn_detail::size(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...)),
            _data(_size, cpp::last_value(s1, sizes...)),
            _dimensions(dyn_detail::sizes(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...)) {
        //Nothing to init
    }

    //Sizes followed by a generator_expr
    template<typename S1, typename... S, cpp_enable_if(
            (sizeof...(S) == D),
            std::is_convertible<std::size_t, typename cpp::first_type<S1, S...>::type>::value,   //The first type must be convertible to size_t
            cpp::is_sub_homogeneous<S1, S...>::value,                                            //The first N-1 types must homegeneous
            cpp::is_specialization_of<generator_expr, typename cpp::last_type<S1, S...>::type>::value     //The last type must be a generator expr
        )>
    dyn_matrix_impl(S1 s1, S... sizes) :
            _size(dyn_detail::size(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...)),
            _data(_size),
            _dimensions(dyn_detail::sizes(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...)) {
        const auto& e = cpp::last_value(sizes...);

        assign_evaluate(e, *this);
    }

    //Sizes followed by an init flag followed by the value
    template<typename... S, cpp_enable_if(dyn_detail::is_init_constructor<S...>::value)>
    dyn_matrix_impl(S... sizes) :
            _size(dyn_detail::size(std::make_index_sequence<(sizeof...(S)-2)>(), sizes...)),
            _data(_size, cpp::last_value(sizes...)),
            _dimensions(dyn_detail::sizes(std::make_index_sequence<(sizeof...(S)-2)>(), sizes...)) {
        static_assert(sizeof...(S) == D + 2, "Invalid number of dimensions");

        //Nothing to init
    }

    template<typename E, cpp_enable_if(
        std::is_convertible<value_t<E>, value_type>::value,
        is_copy_expr<E>::value
    )>
    dyn_matrix_impl(E&& e) :_size(etl::size(e)), _data(_size) {
        for(std::size_t d = 0; d < etl::dimensions(e); ++d){
            _dimensions[d] = etl::dim(e, d);
        }

        assign_evaluate(std::forward<E>(e), *this);
    }

    template<typename Container, cpp_enable_if(
        cpp::not_c<is_etl_expr<Container>>::value,
        std::is_convertible<typename Container::value_type, value_type>::value
    )>
    dyn_matrix_impl(const Container& vec) : _size(vec.size()), _data(_size), _dimensions{{_size}} {
        static_assert(D == 1, "Only 1D matrix can be constructed from containers");

        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = vec[i];
        }
    }

    //Default move constructor
    dyn_matrix_impl(dyn_matrix_impl&& rhs) = default;

    //}}}

    //{{{ Assignment

    //Copy assignment operator

    dyn_matrix_impl& operator=(const dyn_matrix_impl& rhs){
        ensure_same_size(*this, rhs);

        std::copy(rhs.begin(), rhs.end(), begin());

        return *this;
    }

    //Construct from expression

    template<typename E, cpp_enable_if(std::is_convertible<value_t<E>, value_type>::value && is_copy_expr<E>::value)>
    dyn_matrix_impl& operator=(E&& e){
        ensure_same_size(*this, e);

        assign_evaluate(e, *this);

        return *this;
    }

    template<typename Generator>
    dyn_matrix_impl& operator=(generator_expr<Generator>&& e){
        assign_evaluate(e, *this);

        return *this;
    }

    //Allow copy from other containers

    template<typename Container, cpp_enable_if(!is_etl_expr<Container>::value, std::is_convertible<typename Container::value_type, value_type>::value)>
    dyn_matrix_impl& operator=(const Container& vec){
        cpp_assert(vec.size() == size(), "Cannot copy from a vector of different size");

        std::copy(vec.begin(), vec.end(), begin());

        return *this;
    }

    //Set the same value to each element of the matrix
    dyn_matrix_impl& operator=(const value_type& value){
        std::fill(_data.begin(), _data.end(), value);

        return *this;
    }

    //Default move assignment operator
    dyn_matrix_impl& operator=(dyn_matrix_impl&& rhs) = default;

    //}}}

    //{{{ Swap operations

    void swap(dyn_matrix_impl& other){
        //TODO This should be relaxed...
        cpp_assert(other.size() == size(), "Cannot swap from a dyn_matrix of different size");

        using std::swap;
        swap(_data, other._data);
        swap(_dimensions, other._dimensions);
    }

    //}}}

    //{{{ Accessors

    std::size_t size() const noexcept {
        return _size;
    }

    std::size_t rows() const noexcept {
        return _dimensions[0];
    }

    std::size_t columns() const noexcept {
        static_assert(n_dimensions > 1, "columns() only valid for 2D+ matrices");
        return _dimensions[1];
    }

    static constexpr std::size_t dimensions() noexcept {
        return n_dimensions;
    }

    std::size_t dim(std::size_t d) const  noexcept {
        cpp_assert(d < n_dimensions, "Invalid dimension");

        return _dimensions[d];
    }

    template<std::size_t D2>
    std::size_t dim() const  noexcept {
        cpp_assert(D2 < n_dimensions, "Invalid dimension");

        return _dimensions[D2];
    }

    template<bool B = (n_dimensions > 1), cpp::enable_if_u<B> = cpp::detail::dummy>
    auto operator()(std::size_t i) noexcept {
        return sub(*this, i);
    }

    template<bool B = (n_dimensions > 1), cpp::enable_if_u<B> = cpp::detail::dummy>
    auto operator()(std::size_t i) const noexcept {
        return sub(*this, i);
    }

    template<bool B = n_dimensions == 1, cpp::enable_if_u<B> = cpp::detail::dummy>
    value_type& operator()(std::size_t i) noexcept {
        cpp_assert(i < dim(0), "Out of bounds");

        return _data[i];
    }

    template<bool B = n_dimensions == 1, cpp::enable_if_u<B> = cpp::detail::dummy>
    const value_type& operator()(std::size_t i) const noexcept {
        cpp_assert(i < dim(0), "Out of bounds");

        return _data[i];
    }

    template<bool B = n_dimensions == 2, cpp::enable_if_u<B> = cpp::detail::dummy>
    value_type& operator()(std::size_t i, std::size_t j) noexcept {
        cpp_assert(i < dim(0), "Out of bounds");
        cpp_assert(j < dim(1), "Out of bounds");

        if(storage_order == order::RowMajor){
            return _data[i * dim(1) + j];
        } else {
            return _data[j * dim(0) + i];
        }
    }

    template<bool B = n_dimensions == 2, cpp::enable_if_u<B> = cpp::detail::dummy>
    const value_type& operator()(std::size_t i, std::size_t j) const noexcept {
        cpp_assert(i < dim(0), "Out of bounds");
        cpp_assert(j < dim(1), "Out of bounds");

        if(storage_order == order::RowMajor){
            return _data[i * dim(1) + j];
        } else {
            return _data[j * dim(0) + i];
        }
    }

    template<typename... S, cpp::enable_if_u<(sizeof...(S) > 0 && storage_order == order::RowMajor)> = cpp::detail::dummy>
    std::size_t index(S... sizes) const noexcept {
        //Note: Version with sizes moved to a std::array and accessed with
        //standard loop may be faster, but need some stack space (relevant ?)

        auto subsize = size();
        std::size_t index = 0;
        std::size_t i = 0;

        cpp::for_each_in(
            [&subsize, &index, &i, this](std::size_t s){
                subsize /= dim(i++);
                index += subsize * s;
            }, sizes...);

        return index;
    }

    template<typename... S, cpp::enable_if_u<(sizeof...(S) > 0 && storage_order == order::ColumnMajor)> = cpp::detail::dummy>
    std::size_t index(S... sizes) const noexcept {
        //Note: Version with sizes moved to a std::array and accessed with
        //standard loop may be faster, but need some stack space (relevant ?)

        auto subsize = 1;
        std::size_t index = 0;
        std::size_t i = 0;

        cpp::for_each_in(
            [&subsize, &index, &i, this](std::size_t s){
                index += subsize * s;
                subsize *= dim(i++);
            }, sizes...);

        return index;
    }

    template<typename... S, cpp::enable_if_all_u<
            (n_dimensions > 2),
            (sizeof...(S) == n_dimensions),
            cpp::all_convertible_to<std::size_t, S...>::value
        > = cpp::detail::dummy>
    const value_type& operator()(S... sizes) const noexcept {
        static_assert(sizeof...(S) == n_dimensions, "Invalid number of parameters");

        return _data[index(sizes...)];
    }

    template<typename... S, cpp::enable_if_all_u<
            (n_dimensions > 2),
            (sizeof...(S) == n_dimensions),
            cpp::all_convertible_to<std::size_t, S...>::value
        > = cpp::detail::dummy>
    value_type& operator()(S... sizes) noexcept {
        static_assert(sizeof...(S) == n_dimensions, "Invalid number of parameters");

        return _data[index(sizes...)];
    }

    const value_type& operator[](std::size_t i) const noexcept {
        cpp_assert(i < size(), "Out of bounds");

        return _data[i];
    }

    value_type& operator[](std::size_t i) noexcept {
        cpp_assert(i < size(), "Out of bounds");

        return _data[i];
    }

    vec_type load(std::size_t i) const noexcept {
        return vec::loadu(memory_start() + i);
    }

    iterator begin() noexcept(noexcept(_data.begin())) {
        return _data.begin();
    }

    iterator end() noexcept(noexcept(_data.end())) {
        return _data.end();
    }

    const_iterator begin() const noexcept(noexcept(_data.cbegin())) {
        return _data.cbegin();
    }

    const_iterator end() const noexcept(noexcept(_data.cend())) {
        return _data.cend();
    }

    const_iterator cbegin() const noexcept(noexcept(_data.cbegin())) {
        return _data.cbegin();
    }

    const_iterator cend() const noexcept(noexcept(_data.cend())) {
        return _data.cend();
    }

    //}}}

    //{{{ Direct memory access

    memory_type memory_start() noexcept {
        return &_data[0];
    }

    const_memory_type memory_start() const noexcept {
        return &_data[0];
    }

    memory_type memory_end() noexcept {
        return &_data[size()];
    }

    const_memory_type memory_end() const noexcept {
        return &_data[size()];
    }

    //}}}

    std::size_t& unsafe_dimension_access(std::size_t i){
        cpp_assert(i < n_dimensions, "Out of bounds");
        return _dimensions[i];
    }
};

template<typename T, order SO, std::size_t D>
void swap(dyn_matrix_impl<T, SO, D>& lhs, dyn_matrix_impl<T, SO, D>& rhs){
    lhs.swap(rhs);
}

template<typename T, order SO, std::size_t D>
std::ostream& operator<<(std::ostream& os, const dyn_matrix_impl<T, SO, D>& mat){
    if(D == 1){
        return os << "V[" << mat.size() << "]";
    } else {
        os << "M[" << mat.dim(0);

        for(std::size_t i = 1; i < D; ++i){
            os << "," << mat.dim(i);
        }

        return os << "]";
    }
}

} //end of namespace etl

#endif