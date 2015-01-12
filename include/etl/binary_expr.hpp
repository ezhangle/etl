//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_BINARY_EXPR_HPP
#define ETL_BINARY_EXPR_HPP

#include "traits_fwd.hpp"
#include "iterator.hpp"

namespace etl {

template <typename T, typename LeftExpr, typename BinaryOp, typename RightExpr>
class binary_expr final {
private:
    static_assert(cpp::or_c<
        cpp::and_c<is_etl_expr<LeftExpr>, std::is_same<RightExpr, scalar<T>>>,
        cpp::and_c<is_etl_expr<RightExpr>, std::is_same<LeftExpr, scalar<T>>>,
        cpp::and_c<is_etl_expr<LeftExpr>, is_etl_expr<RightExpr>>>::value,
        "One argument must be an ETL expression and the other one convertible to T");

    using this_type = binary_expr<T, LeftExpr, BinaryOp, RightExpr>;

    LeftExpr _lhs;
    RightExpr _rhs;

public:
    using value_type = T;

    //Cannot be constructed with no args
    binary_expr() = delete;

    //Construct a new expression
    binary_expr(LeftExpr l, RightExpr r) : _lhs(std::forward<LeftExpr>(l)), _rhs(std::forward<RightExpr>(r)) {
        //Nothing else to init
    }

    //Copy an expression
    binary_expr(const binary_expr& e) : _lhs(e._lhs), _rhs(e._rhs) {
        //Nothing else to init
    }

    //Move an expression
    binary_expr(binary_expr&& e) : _lhs(e._lhs), _rhs(e._rhs) {
        //Nothing else to init
    }

    //Expressions are invariant
    binary_expr& operator=(const binary_expr&) = delete;
    binary_expr& operator=(binary_expr&&) = delete;

    //Accessors

    std::add_lvalue_reference_t<LeftExpr> lhs(){
        return _lhs;
    }

    cpp::add_const_lvalue_t<LeftExpr> lhs() const {
        return _lhs;
    }

    std::add_lvalue_reference_t<RightExpr> rhs(){
        return _rhs;
    }

    cpp::add_const_lvalue_t<RightExpr> rhs() const {
        return _rhs;
    }

    //Apply the expression

    value_type operator[](std::size_t i) const {
        return BinaryOp::apply(lhs()[i], rhs()[i]);
    }

    template<typename... S>
    std::enable_if_t<sizeof...(S) == sub_size_compare<this_type>::value, value_type> operator()(S... args) const {
        static_assert(cpp::all_convertible_to<std::size_t, S...>::value, "Invalid size types");

        return BinaryOp::apply(lhs()(args...), rhs()(args...));
    }

    template<bool B = (sub_size_compare<this_type>::value > 1), cpp::enable_if_u<B> = cpp::detail::dummy>
    auto operator()(std::size_t i){
        return sub(*this, i);
    }

    template<bool B = (sub_size_compare<this_type>::value > 1), cpp::enable_if_u<B> = cpp::detail::dummy>
    auto operator()(std::size_t i) const {
        return sub(*this, i);
    }

    iterator<const this_type> begin() const noexcept {
        return {*this, 0};
    }

    iterator<const this_type> end() const noexcept {
        return {*this, size(*this)};
    }
};

} //end of namespace etl

#endif
