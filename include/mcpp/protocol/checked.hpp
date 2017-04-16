/**
 *	\file
 */

#pragma once

#include "../checked.hpp"
#include "error.hpp"
#include <boost/expected/expected.hpp>
#include <cstddef>
#include <system_error>

namespace mcpp {
namespace protocol {
namespace checked {

/**
 *	Functions identically to \ref mcpp::checked::cast
 *	except that it returns a \ref error which wraps
 *	\ref error_code::overflow on failure.
 *
 *	\tparam To
 *		The integer type to convert to.
 *	\tparam From
 *		The integer or optional type to convert from.
 *
 *	\param [in] val
 *		The value to convert.
 *	\param [in] offset
 *		The offset at which \em val was parsed to be
 *		used to construct an \ref error object in case
 *		of failure.
 *
 *	\return
 *		The integer if conversion succeeds. If conversion
 *		fails a \ref error object.
 */
template <typename To, typename From>
boost::expected<To, std::error_code> cast (const From & val) noexcept {
	//	TODO: Handle unwrapping boost::expected
	//	objects in addition to the optional
	//	unwrapping the underlying implementation
	//	performs
	auto o = mcpp::checked::cast<To>(val);
	if (!o) return boost::make_unexpected(make_error_code(error::overflow));
	return *o;
}

}
}
}
