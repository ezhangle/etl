//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_FAST_HPP
#define ETL_FAST_HPP

#include<string>

#include "cpp_utils/assert.hpp"

#include "tmp.hpp"
#include "traits_fwd.hpp"

namespace etl {

namespace matrix_detail {

template<typename M, std::size_t I, typename Enable = void>
struct matrix_subsize  : std::integral_constant<std::size_t, M::template dim<I+1>() * matrix_subsize<M, I+1>::value> {};

template<typename M, std::size_t I>
struct matrix_subsize<M, I, std::enable_if_t<I == M::n_dimensions - 1>> : std::integral_constant<std::size_t, 1> {};

template<typename M, std::size_t I, std::size_t Stop, typename S1, typename... S>
struct matrix_index {
    template<std::size_t I2, typename Enable = void>
    struct matrix_index_int {
        static std::size_t compute(S1 first, S... args){
            cpp_assert(first < M::template dim<I2>(), "Out of bounds");

            return matrix_subsize<M, I>::value * first + matrix_index<M, I+1, Stop, S...>::compute(args...);
        }
    };

    template<std::size_t I2>
    struct matrix_index_int<I2, std::enable_if_t<I2 == Stop>> {
        static std::size_t compute(S1 first){
            cpp_assert(first < M::template dim<I2>(), "Out of bounds");

            return first;
        }
    };

    static std::size_t compute(S1 first, S... args){
        return matrix_index_int<I>::compute(first, args...);
    }
};

template <typename N>
struct is_vector : std::false_type { };

template <typename N, typename A>
struct is_vector<std::vector<N, A>> : std::true_type { };

template <typename N>
struct is_vector<std::vector<N>> : std::true_type { };

} //end of namespace detail

template<typename T, typename ST, std::size_t... Dims>
struct fast_matrix_impl final {
    static_assert(sizeof...(Dims) > 0, "At least one dimension must be specified");

public:
    static constexpr const std::size_t n_dimensions = sizeof...(Dims);
    static constexpr const std::size_t etl_size = mul_all<Dims...>::value;
    static constexpr const bool array_impl = !matrix_detail::is_vector<ST>::value;

    using       value_type = T;
    using     storage_impl = ST;
    using         iterator = typename storage_impl::iterator;
    using   const_iterator = typename storage_impl::const_iterator;
    using        this_type = fast_matrix_impl<T, ST, Dims...>;

private:
    storage_impl _data;

    template<typename... S>
    static constexpr std::size_t index(S... args){
        return matrix_detail::matrix_index<this_type, 0, n_dimensions - 1, S...>::compute(args...);
    }

    template<typename... S>
    value_type& access(S... args){
        return _data[index(args...)];
    }

    template<typename... S>
    const value_type& access(S... args) const {
        return _data[index(args...)];
    }

public:

    ///{{{ Construction

    template<typename S = ST, cpp::enable_if_c<matrix_detail::is_vector<S>> = cpp::detail::dummy>
    void init(){
        _data.resize(etl_size);
    }

    template<typename S = ST, cpp::disable_if_c<matrix_detail::is_vector<S>> = cpp::detail::dummy>
    void init() noexcept {
        //Nothing to init
    }

    fast_matrix_impl() noexcept(array_impl) {
        init();
    }

    template<typename VT, cpp::enable_if_one_c<std::is_convertible<VT, value_type>, std::is_assignable<T&, VT>> = cpp::detail::dummy>
    explicit fast_matrix_impl(const VT& value){
        init();
        std::fill(_data.begin(), _data.end(), value);
    }

    fast_matrix_impl(std::initializer_list<value_type> l){
        init();

        cpp_assert(l.size() == size(), "Cannot copy from an initializer of different size");

        std::copy(l.begin(), l.end(), begin());
    }

    fast_matrix_impl(const fast_matrix_impl& rhs) noexcept(array_impl) {
        init();
        std::copy(rhs.begin(), rhs.end(), begin());
    }

    template<typename S = ST, cpp::enable_if_c<matrix_detail::is_vector<S>> = cpp::detail::dummy>
    fast_matrix_impl(fast_matrix_impl&& rhs) noexcept {
        _data = std::move(rhs._data);
    }

    template<typename S = ST, cpp::disable_if_c<matrix_detail::is_vector<S>> = cpp::detail::dummy>
    explicit fast_matrix_impl(fast_matrix_impl&& rhs) noexcept {
        std::copy(rhs.begin(), rhs.end(), begin());
    }

    template<typename E, cpp::enable_if_all_c<std::is_convertible<typename E::value_type, value_type>, is_copy_expr<E>> = cpp::detail::dummy>
    explicit fast_matrix_impl(const E& e){
        init();
        ensure_same_size(*this, e);

        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = e[i];
        }
    }

    template<typename Container, cpp::enable_if_all_c<
        std::is_convertible<typename Container::value_type, value_type>,
        cpp::not_c<is_copy_expr<Container>>
    > = cpp::detail::dummy>
    explicit fast_matrix_impl(const Container& vec){
        init();
        cpp_assert(vec.size() == size(), "Cannnot copy from a container of another size");

        std::copy(vec.begin(), vec.end(), begin());
    }

    template<typename Generator>
    explicit fast_matrix_impl(generator_expr<Generator>&& e){
        init();
        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = e[i];
        }
    }

    //}}}

    //{{{ Assignment

    //Copy assignment operator

    fast_matrix_impl& operator=(const fast_matrix_impl& rhs) noexcept {
        std::copy(rhs.begin(), rhs.end(), begin());

        return *this;
    }

    //Allow copy from other containers

    template<typename Container, cpp::enable_if_c<std::is_convertible<typename Container::value_type, value_type>> = cpp::detail::dummy>
    fast_matrix_impl& operator=(const Container& vec) noexcept {
        std::copy(vec.begin(), vec.end(), begin());

        return *this;
    }

    //Construct from expression

    template<typename E, cpp::enable_if_all_c<std::is_convertible<typename E::value_type, value_type>, is_copy_expr<E>> = cpp::detail::dummy>
    fast_matrix_impl& operator=(E&& e){
        ensure_same_size(*this, e);

        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = e[i];
        }

        return *this;
    }

    template<typename Generator>
    fast_matrix_impl& operator=(generator_expr<Generator>&& e){
        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = e[i];
        }

        return *this;
    }

    //Set the same value to each element of the matrix
    template<typename VT, cpp::enable_if_one_c<std::is_convertible<VT, value_type>, std::is_assignable<T&, VT>> = cpp::detail::dummy>
    fast_matrix_impl& operator=(const VT& value) noexcept {
        std::fill(_data.begin(), _data.end(), value);

        return *this;
    }

    //Prohibit move
    fast_matrix_impl& operator=(fast_matrix_impl&& rhs) = delete;

    //}}}

    void swap(fast_matrix_impl& other){
        using std::swap;
        swap(_data, other._data);
    }

    //{{{ Accessors

    static constexpr std::size_t size() noexcept {
        return etl_size;
    }

    static constexpr std::size_t rows() noexcept {
        return dim<0>();
    }

    static constexpr std::size_t columns() noexcept {
        static_assert(n_dimensions > 1, "columns() can only be used on 2D+ matrices");

        return dim<1>();
    }

    static constexpr std::size_t dimensions() noexcept {
        return n_dimensions;
    }

    template<std::size_t D>
    static constexpr std::size_t dim() noexcept {
        return nth_size<D, 0, Dims...>::value;
    }

    bool is_finite() const noexcept(noexcept(this->begin())) {
        return std::find_if(begin(), end(), [](auto& v){return !std::isfinite(v);}) == end();
    }

    //TODO Would probably be useful to have dim(std::size_t i)

    template<bool B = (n_dimensions > 1), cpp::enable_if_u<B> = cpp::detail::dummy>
    auto operator()(std::size_t i) noexcept {
        return sub(*this, i);
    }

    template<bool B = (n_dimensions > 1), cpp::enable_if_u<B> = cpp::detail::dummy>
    auto operator()(std::size_t i) const noexcept {
        return sub(*this, i);
    }

    template<typename... S>
    std::enable_if_t<sizeof...(S) == sizeof...(Dims), value_type&> operator()(S... args) noexcept {
        static_assert(cpp::all_convertible_to<std::size_t, S...>::value, "Invalid size types");

        return access(static_cast<std::size_t>(args)...);
    }

    template<typename... S>
    std::enable_if_t<sizeof...(S) == sizeof...(Dims), const value_type&> operator()(S... args) const noexcept {
        static_assert(cpp::all_convertible_to<std::size_t, S...>::value, "Invalid size types");

        return access(static_cast<std::size_t>(args)...);
    }

    const value_type& operator[](std::size_t i) const noexcept {
        cpp_assert(i < size(), "Out of bounds");

        return _data[i];
    }

    value_type& operator[](std::size_t i) noexcept {
        cpp_assert(i < size(), "Out of bounds");

        return _data[i];
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

    const_iterator end() const noexcept(noexcept(_data.end())) {
        return _data.cend();
    }

    const_iterator cbegin() const noexcept(noexcept(_data.cbegin())) {
        return _data.cbegin();
    }

    const_iterator cend() const noexcept(noexcept(_data.end())) {
        return _data.cend();
    }

    //}}}
};

template<typename T, typename ST, std::size_t... Dims>
void swap(fast_matrix_impl<T, ST, Dims...>& lhs, fast_matrix_impl<T, ST, Dims...>& rhs){
    lhs.swap(rhs);
}

} //end of namespace etl

#endif