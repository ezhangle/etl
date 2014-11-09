//=======================================================================
// Copyright (c) 2014 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_ETL_HPP
#define ETL_ETL_HPP

// The operators
#include "generators.hpp"
#include "transformers.hpp"
#include "views.hpp"
#include "unary_op.hpp"
#include "binary_op.hpp"

// The expressions
#include "binary_expr.hpp"
#include "unary_expr.hpp"
#include "unstable_transform_expr.hpp"
#include "stable_transform_expr.hpp"
#include "generator_expr.hpp"

// The value classes
#include "etl/fast_matrix.hpp"
#include "etl/fast_vector.hpp"
#include "etl/dyn_matrix.hpp"
#include "etl/dyn_vector.hpp"

// The traits
#include "traits.hpp"

// The expressions building
#include "fast_expr.hpp"

#endif