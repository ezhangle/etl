//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

namespace etl {

/*!
 * \brief Wrapper used in the context to force an implementation to be used
 */
template <typename T>
struct forced_impl {
    T impl;              ///< The impl to be used (if forced == true)
    bool forced = false; ///< Indicate if forced or default
};

/*!
 * \brief The contextual configuration of ETL
 */
struct context {
    bool serial   = false; ///< Force serial execution
    bool parallel = false; ///< Force parallel execution

    forced_impl<scalar_impl> scalar_selector;         ///< Force selector for scalar operations
    forced_impl<sum_impl> sum_selector;               ///< Forced selector for sum
    forced_impl<transpose_impl> transpose_selector;   ///< Forced selector for transpose
    forced_impl<dot_impl> dot_selector;               ///< Forced selector for dot
    forced_impl<conv_impl> conv_selector;             ///< Forced selector for conv
    forced_impl<conv_multi_impl> conv_multi_selector; ///< Forced selector for conv_multi
    forced_impl<conv4_impl> conv4_selector;           ///< Forced selector for conv4
    forced_impl<gemm_impl> gemm_selector;             ///< Forced selector for gemm
    forced_impl<outer_impl> outer_selector;           ///< Forced selector for outer product
    forced_impl<fft_impl> fft_selector;               ///< Forced selector for fft
};

/*!
 * \brief Return the configuration context of the current thread.
 * \return the configuration context of the current thread
 */
inline context& local_context() {
    static thread_local context local_context;
    return local_context;
}

namespace detail {

/*!
 * \brief Return the forced_impl of the local context for the given enumeration type
 * \tparam T The type of enumeration of implmentation
 * \return the forced_impl of the given type for the local context
 */
template <typename T>
forced_impl<T>& get_forced_impl();

/*!
 * \copydoc get_forced_impl
 */
template <>
inline forced_impl<scalar_impl>& get_forced_impl() {
    return local_context().scalar_selector;
}

/*!
 * \copydoc get_forced_impl
 */
template <>
inline forced_impl<sum_impl>& get_forced_impl() {
    return local_context().sum_selector;
}

/*!
 * \copydoc get_forced_impl
 */
template <>
inline forced_impl<transpose_impl>& get_forced_impl() {
    return local_context().transpose_selector;
}

/*!
 * \copydoc get_forced_impl
 */
template <>
inline forced_impl<dot_impl>& get_forced_impl() {
    return local_context().dot_selector;
}

/*!
 * \copydoc get_forced_impl
 */
template <>
inline forced_impl<conv_impl>& get_forced_impl() {
    return local_context().conv_selector;
}

/*!
 * \copydoc get_forced_impl
 */
template <>
inline forced_impl<conv_multi_impl>& get_forced_impl() {
    return local_context().conv_multi_selector;
}

/*!
 * \copydoc get_forced_impl
 */
template <>
inline forced_impl<conv4_impl>& get_forced_impl() {
    return local_context().conv4_selector;
}

/*!
 * \copydoc get_forced_impl
 */
template <>
inline forced_impl<gemm_impl>& get_forced_impl() {
    return local_context().gemm_selector;
}

/*!
 * \copydoc get_forced_impl
 */
template <>
inline forced_impl<outer_impl>& get_forced_impl() {
    return local_context().outer_selector;
}

/*!
 * \copydoc get_forced_impl
 */
template <>
inline forced_impl<fft_impl>& get_forced_impl() {
    return local_context().fft_selector;
}

/*!
 * \brief RAII helper for setting the context to serial
 */
struct serial_context {
    bool old_serial; ///< The previous value of serial

    /*!
     * \brief Default construct a serial context
     *
     * This saves the previous serial value and sets serial to true
     */
    serial_context() {
        old_serial = etl::local_context().serial;
        etl::local_context().serial = true;
    }

    /*!
     * \brief Destruct a serial context
     *
     * This restores the serial state
     */
    ~serial_context() {
        etl::local_context().serial = old_serial;
    }

    /*!
     * \brief Does nothing, simple trick for section to be nice
     */
    operator bool() {
        return true;
    }
};

/*!
 * \brief RAII helper for setting the context to parallel
 */
struct parallel_context {
    bool old_parallel; ///< The previous value of parallel

    /*!
     * \brief Default construct a parallel context
     *
     * This saves the previous parallel value and sets parallel to true
     */
    parallel_context() {
        old_parallel = etl::local_context().parallel;
        etl::local_context().parallel = true;
    }

    /*!
     * \brief Destruct a parallel context
     *
     * This restores the parallel state
     */
    ~parallel_context() {
        etl::local_context().parallel = old_parallel;
    }

    /*!
     * \brief Does nothing, simple trick for section to be nice
     */
    operator bool() {
        return true;
    }
};

/*!
 * \brief RAII helper for setting the context to a selected
 * implementation
 */
template <typename Selector, Selector V>
struct selected_context {
    forced_impl<Selector> old_selector; ///< The previous value of selector

    /*!
     * \brief Default construct a selected context
     *
     * This saves the previous selector value and sets selector to
     * the specified implementation.
     */
    selected_context() {
        decltype(auto) selector = get_forced_impl<Selector>();

        old_selector = selector;

        selector.impl   = V;
        selector.forced = true;
    }

    /*!
     * \brief Destruct a selected context
     *
     * This restores the selector state
     */
    ~selected_context() {
        get_forced_impl<Selector>() = old_selector;
    }

    /*!
     * \brief Does nothing, simple trick for section to be nice
     */
    operator bool() {
        return true;
    }
};

} //end of namespace detail

/*!
 * \brief Define the start of an ETL serial section
 */
#define SERIAL_SECTION if (auto etl_serial_context__ = etl::detail::serial_context())

/*!
 * \brief Define the start of an ETL parallel section
 */
#define PARALLEL_SECTION if (auto etl_parallel_context__ = etl::detail::parallel_context())

/*!
 * \brief Define the start of an ETL selected section
 */
#define SELECTED_SECTION(v) if (auto etl_selected_context__ = etl::detail::selected_context<decltype(v), v>())

} //end of namespace etl
