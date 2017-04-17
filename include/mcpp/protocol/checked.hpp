/**
 *	\file
 */

#pragma once

#include "../checked.hpp"
#include "../optional.hpp"
#include "error.hpp"
#include <boost/expected/expected.hpp>
#include <cstddef>
#include <system_error>
#include <type_traits>

namespace mcpp {
namespace protocol {
namespace checked {

namespace detail {

template <typename T>
boost::expected<T, std::error_code> convert (const optional<T> & opt) noexcept {
	if (!opt) return boost::make_unexpected(make_error_code(error::overflow));
	return *opt;
}

}

/**
 *	Functions identically to \ref mcpp::checked::cast
 *	except that it returns a `std::error_code` which wraps
 *	\ref error::overflow on failure.
 *
 *	\tparam To
 *		The integer type to convert to.
 *	\tparam From
 *		The integer or optional type to convert from.
 *
 *	\param [in] val
 *		The value to convert.
 *
 *	\return
 *		The integer if conversion succeeds. If conversion
 *		fails a `std::error_code`.
 */
template <typename To, typename From>
boost::expected<To, std::error_code> cast (const From & val) noexcept {
	//	TODO: Handle unwrapping boost::expected
	//	objects in addition to the optional
	//	unwrapping the underlying implementation
	//	performs
	return detail::convert(mcpp::checked::cast<To>(val));
}

/**
 *	Functions identically to \ref mcpp::checked::add
 *	except that it returns a `std::error_code` which
 *	wraps \ref error::overflow on failure.
 *
 *	\tparam Ts
 *		A parameter pack containing the types of integers
 *		to add.
 *
 *	\param [in] args
 *		A pack of integers to add.
 *
 *	\return
 *		The result if addition succeeds. `std::error_code`
 *		otherwise.
 */
template <typename... Ts>
boost::expected<typename std::result_of_t<mcpp::checked::add(Ts...)>::value_type, std::error_code> add (const Ts &... args) noexcept {
	return detail::convert(mcpp::checked::add(args...));
}

}
}
}
