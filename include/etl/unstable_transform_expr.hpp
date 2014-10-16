//=======================================================================
// Copyright (c) 2014 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_UNSTABLE_TRANSFORM_EXPR_HPP
#define ETL_UNSTABLE_TRANSFORM_EXPR_HPP

#include "traits.hpp"

namespace etl {

template <typename T, typename Expr>
class unstable_transform_expr {
private:
    static_assert(is_etl_expr<Expr>::value, "Only ETL expressions can be used in unstable_transform_expr");

    using this_type = unstable_transform_expr<T, Expr>;

    Expr _value;
 
public:
    using value_type = T;
    using expr_type = Expr;

    //Cannot be constructed with no args
    unstable_transform_expr() = delete;

    //Construct a new expression
    unstable_transform_expr(Expr l) : _value(std::forward<Expr>(l)){
        //Nothing else to init
    }

    unstable_transform_expr(const unstable_transform_expr& e) : _value(e._value) {
        //Nothing else to init
    }

    unstable_transform_expr(unstable_transform_expr&& e) : _value(e._value) {
        //Nothing else to init
    }

    //Expression are invariant
    unstable_transform_expr& operator=(const unstable_transform_expr&) = delete;
    unstable_transform_expr& operator=(unstable_transform_expr&&) = delete;

    //Accessors

    typename std::add_lvalue_reference<Expr>::type value(){
        return _value;
    }

    typename std::add_lvalue_reference<typename std::add_const<Expr>::type>::type value() const {
        return _value;
    }

    //Apply the expression

    value_type operator[](std::size_t i) const {
        return value()[i];
    }

    template<typename... S>
    std::enable_if_t<sizeof...(S) == sub_size_compare<this_type>::value, value_type> operator()(S... args) const {
        static_assert(cpp::all_convertible_to<std::size_t, S...>::value, "Invalid size types");

        return value()(args...);
    }
};

//}}}

} //end of namespace etl

#endif