//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_COMPAT_HPP
#define ETL_COMPAT_HPP

//Workaround for use of this in noexcept specifier

#ifdef __clang__
#define noexcept_this(...) noexcept( __VA_ARGS__ )
#else
//GCC is bugged (Bug: 52869), it is therefore impossible to use this in noexcept
#define noexcept_this(...)
#endif

#endif