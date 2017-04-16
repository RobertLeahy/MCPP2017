/**
 *	\file
 */

#pragma once

#include "optional.hpp"
#include <limits>
#include <type_traits>
#include <utility>

namespace mcpp {
namespace checked {

namespace detail {

template <typename To, typename From>
constexpr bool cast_max_always_safe_v = std::numeric_limits<To>::max() >= std::numeric_limits<From>::max();
template <typename To, typename From>
using cast_max_always_safe_t = std::integral_constant<bool, cast_max_always_safe_v<To, From>>;
template <typename To, typename From>
constexpr bool cast_min_always_safe_v =
(std::is_unsigned<To>::value && std::is_unsigned<From>::value) ||
(std::is_signed<To>::value && std::is_unsigned<From>::value) ||
(
	std::is_signed<To>::value && std::is_signed<From>::value &&
	(std::numeric_limits<To>::min() == std::numeric_limits<From>::min())
);
template <typename To, typename From>
using cast_min_always_safe_t = std::integral_constant<bool, cast_min_always_safe_v<To, From>>;

template <typename To, typename From>
constexpr bool cast_check_max (From val, const std::false_type &) noexcept {
	return val <= std::numeric_limits<To>::max();
}
template <typename, typename From>
constexpr bool cast_check_max (const From &, const std::true_type &) noexcept {
	return true;
}

template <typename To, typename From>
constexpr bool cast_check_min_impl (From val, const std::true_type &) noexcept {
	//	Being here means both To and From
	//	are signed, which means we need
	//	to actually check the minimum
	return val >= std::numeric_limits<To>::min();
}
template <typename To, typename From>
constexpr bool cast_check_min_impl (From val, const std::false_type &) noexcept {
	//	Being here means To is unsigned
	//	and From is signed, which means
	//	if the value is negative we're
	//	hosed
	return val >= 0;
}

template <typename To, typename From>
constexpr bool cast_check_min (From val, const std::false_type &) noexcept {
	//	There are four possibilities:
	//
	//	-	Signed/Signed: This function gets called
	//	-	Unsigned/Unsigned: This function does not get called
	//	-	Signed/Unsigned: This function does not get called
	//	-	Unsigned/Signed: This function gets called
	//
	//	Therefore one boolean is enough to differentiate
	//	between the different cases
	typename std::is_signed<To>::type tag;
	return detail::cast_check_min_impl<To>(val, tag);
}
template <typename To, typename From>
constexpr bool cast_check_min (const From &, const std::true_type &) noexcept {
	return true;
}

}

/**
 *	Attempts to convert an integer from one
 *	type to another.
 *
 *	\tparam To
 *		The integer type to convert to.
 *	\tparam From
 *		The integer type to convert from. For
 *		convenience may also be an optional
 *		integer.
 *
 *	\tparam [in] val
 *		The value to attempt to convert. If this
 *		is an optional integer (or an arbitrary
 *		nesting of optionals with an integer as
 *		the innermost type) the conversion will
 *		only proceed if the optional has a value
 *		at the innermost level.
 *
 *	\return
 *		\em val represented as type \em To if
 *		this is possible, an empty optional
 *		otherwise.
 */
template <typename To, typename From>
optional<To> cast (const From & val) noexcept {
	return mcpp::bind_optional(
		[] (auto val) noexcept -> optional<To> {
			using type = decltype(val);
			detail::cast_min_always_safe_t<To, type> min_tag;
			if (!detail::cast_check_min<To>(val, min_tag)) {
				return nullopt;
			}
			detail::cast_max_always_safe_t<To, type> max_tag;
			if (!detail::cast_check_max<To>(val, max_tag)) {
				return nullopt;
			}
			return To(val);
		},
		val
	);
}

namespace detail {

template <typename T>
constexpr bool check_add (T a, T b, const std::false_type &) noexcept {
	return (std::numeric_limits<T>::max() - a) >= b;
}
//	TODO: Add support
template <typename T>
constexpr bool check_add (T, T, const std::true_type &) noexcept;

template <typename T>
constexpr bool check_add (T a, T b) noexcept {
	typename std::is_signed<T>::type tag;
	return detail::check_add(a, b, tag);
}

class add_impl {
public:
	template <typename T, typename U, typename... Ts>
	optional<T> operator () (T a, U b, Ts... ops) const noexcept {
		auto result = checked::cast<T>(b);
		if (!result) return nullopt;
		if (!detail::check_add(a, *result)) return nullopt;
		a += *result;
		return (*this)(a, ops...);
	}
	template <typename T>
	optional<T> operator () (T a) const noexcept {
		return a;
	}
	optional<int> operator () () const noexcept {
		return 0;
	}
};

}

/**
 *	Attempts to safely add integers.
 *
 *	Note that this operation has the following
 *	characteristics:
 *
 *	-	Calling this function with zero values
 *		will result in an `int` value of zero
 *	-	Calling this function with one value
 *		will result in a value of that type
 *		with that value
 *	-	Calling this function with N values
 *		will result in a value of the leftmost
 *		type assuming the operation can be
 *		performed without overflow
 *	-	Operations are always grouped from
 *		left to right and are always performed
 *		using the bounds of the result type
 *		(i.e. the leftmost type)
 *
 *	\tparam Ts
 *		The types of integers to add. For
 *		convenience any or all of these may
 *		be optionals of any nesting level.
 *
 *	\param [in] args
 *		The operands.
 *
 *	\return
 *		An optional result. No result will be
 *		returned if the operation could not be
 *		performed without overflow.
 */
template <typename... Ts>
auto add (const Ts &... args) noexcept {
	detail::add_impl adder;
	return mcpp::bind_optional(adder, args...);
}

namespace detail {

template <typename T>
constexpr bool check_multiply (T a, T b, const std::false_type &) noexcept {
	return (std::numeric_limits<T>::max() / a) >= b;
}
template <typename T>
constexpr bool check_multiply (T, T, const std::true_type &) noexcept;

template <typename T>
constexpr bool check_multiply (T a, T b) noexcept {
	typename std::is_signed<T>::type tag;
	return detail::check_multiply(a, b, tag);
}

class multiply_impl {
public:
	template <typename T, typename U, typename... Ts>
	optional<T> operator () (T a, U b, Ts... ops) const noexcept {
		auto result = checked::cast<T>(b);
		if (!result) return nullopt;
		if (!detail::check_multiply(a, *result)) return nullopt;
		a *= *result;
		return (*this)(a, ops...);
	}
	template <typename T>
	optional<T> operator () (T a) const noexcept {
		return a;
	}
	optional<int> operator () () const noexcept {
		return 0;
	}
};

}

/**
 *	Attempts to safely multiply integers.
 *
 *	Note that this operation has the following
 *	characteristics:
 *
 *	-	Calling this function with zero values
 *		will result in an `int` value of zero
 *	-	Calling this function with one value
 *		will result in a value of that type
 *		with that value
 *	-	Calling this function with N values
 *		will result in a value of the leftmost
 *		type assuming the operation can be
 *		performed without overflow
 *	-	Operations are always grouped from
 *		left to right and are always performed
 *		using the bounds of the result type
 *		(i.e. the leftmost type)
 *
 *	\tparam Ts
 *		The types of integers to multiply. For
 *		convenience any or all of these may
 *		be optionals of any nesting level.
 *
 *	\param [in] args
 *		The operands.
 *
 *	\return
 *		An optional result. No result will be
 *		returned if the operation could not be
 *		performed without overflow.
 */
template <typename... Ts>
auto multiply (const Ts &... args) noexcept {
	detail::multiply_impl multiplier;
	return mcpp::bind_optional(multiplier, args...);
}

}
}
