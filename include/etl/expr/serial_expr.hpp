//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*!
 * \file
 * \brief Contains an expression that is forced to be executed serially
*/

#pragma once

#include "etl/wrapper_traits.hpp"

namespace etl {

/*!
 * \brief A wrapper for expressions that is forced to be serial.
 */
template <typename Expr>
struct serial_expr final {
private:
    Expr _value;

public:
    using expr_t     = Expr;          ///< The wrapped expression type
    using value_type = value_t<Expr>; ///< The value type

    //Cannot be constructed with no args
    serial_expr() = delete;

    /*!
     * \brief Construt a new optimized expression around the given ETL expression
     * \param l The ETL expression
     */
    explicit serial_expr(Expr l)
            : _value(std::forward<Expr>(l)) {
        //Nothing else to init
    }

    //Expresison can be copied and moved
    serial_expr(const serial_expr& e) = default;
    serial_expr(serial_expr&& e) noexcept = default;

    //Expressions are invariant
    serial_expr& operator=(const serial_expr& e) = delete;
    serial_expr& operator=(serial_expr&& e) = delete;

    /*!
     * \brief Return the sub expression
     * \return a reference to the sub expression
     */
    std::add_lvalue_reference_t<Expr> value() {
        return _value;
    }

    /*!
     * \brief Return the sub expression
     * \return a reference to the sub expression
     */
    cpp::add_const_lvalue_t<Expr> value() const {
        return _value;
    }
};

/*!
 * \brief Specilization of the traits for serial_expr
 * Serial expression simply use the same traits as its expression
 */
template <typename Expr>
struct etl_traits<etl::serial_expr<Expr>> : wrapper_traits<etl::serial_expr<Expr>> {};

/*!
 * \brief Prints the type of the optimized expression to the stream
 * \param os The output stream
 * \param expr The expression to print
 * \return the output stream
 */
template <typename Expr>
std::ostream& operator<<(std::ostream& os, const serial_expr<Expr>& expr) {
    return os << "serial(" << expr.value() << ")";
}

} //end of namespace etl
