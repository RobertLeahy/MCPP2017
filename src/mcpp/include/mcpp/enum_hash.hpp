/**
 *	\file
 */

#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>

namespace mcpp {

/**
 *	A specialization of `std::hash` may derived
 *	from this class to automatically implement
 *	hashing functionality for an enumeration.
 *
 *	\tparam T
 *		The enumeration type.
 */
template <typename T>
class enum_hash {
public:
	using result_type = std::size_t;
	using argument_type = T;
	result_type operator () (argument_type e) const noexcept {
		using type = std::underlying_type_t<T>;
		std::hash<type> h;
		return h(static_cast<type>(e));
	}
};

}
