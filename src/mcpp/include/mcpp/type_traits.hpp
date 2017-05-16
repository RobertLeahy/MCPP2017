/**
 *	\file
 */

#pragma once

namespace mcpp {

namespace detail {

template <typename... Args>
class make_void {
public:
	using type = void;
};

}

/**
 *	Polyfill for C++17's `std::void_t`.
 *
 *	\tparam Args
 *		Ignored.
 */
template <typename... Args>
using void_t = typename detail::make_void<Args...>::type;

}
