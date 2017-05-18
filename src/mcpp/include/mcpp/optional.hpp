/**
 *	\file
 */

#pragma once

#ifdef MCPP_HAS_OPTIONAL
#include <optional>
namespace mcpp {
using std::bad_optional_access;
using std::make_optional;
using std::in_place;
using std::in_place_t;
using std::nullopt;
using std::nullopt_t;
using std::optional;
}
#else
#ifdef MCPP_HAS_EXPERIMENTAL_OPTIONAL
#include <experimental/optional>
#else
#include <optional.hpp>
#endif
namespace mcpp {
using std::experimental::bad_optional_access;
using std::experimental::make_optional;
using std::experimental::in_place;
using std::experimental::in_place_t;
using std::experimental::nullopt;
using std::experimental::nullopt_t;
using std::experimental::optional;
}
#endif

#include <algorithm>
#include <type_traits>
#include <utility>

namespace mcpp {

namespace detail {

template <typename T>
class unwrap_optional {
public:
	using type = optional<T>;
};
template <typename T>
class unwrap_optional<optional<T>> {
	using type = typename unwrap_optional<T>::type;
};

template <typename T>
using unwrap_optional_t = typename unwrap_optional<T>::type;

}

/**
 *	Unwraps nested optional objects returning-
 *	the innermost if all of them contain a value,
 *	an empty optional otherwise. If an object
 *	that isn't an optional is passed that object
 *	wrapped in an optional will be returned.
 *
 *	\tparam T
 *		The type to unwrap.
 *
 *	\param [in] obj
 *		The object to unwrap.
 *
 *	\return
 *		An optional containing the innermost
 *		type if there is one, an optional which
 *		does not contain a value otherwise.
 */
template <typename T>
optional<T> unwrap_optional (T obj) noexcept(std::is_nothrow_move_constructible<T>::value) {
	return obj;
}
template <typename T>
detail::unwrap_optional_t<T> unwrap_optional (optional<T> obj) noexcept(std::is_nothrow_move_constructible<T>::value) {
	if (!obj) return nullopt;
	return unwrap_optional(std::move(*obj));
}

namespace detail {

template <typename T>
constexpr bool check_optional (const T &) noexcept {
	return true;
}
template <typename T>
bool check_optional (const optional<T>& opt) noexcept {
	if (opt) return detail::check_optional(*opt);
	return false;
}

template <typename... Ts>
bool check_optionals (Ts &&... args) noexcept {
	return std::min({detail::check_optional(args)...});
}
constexpr bool check_optionals () noexcept {
	return true;
}

template <typename T>
decltype(auto) deref_optional (T && val) noexcept {
	return std::forward<T>(val);
}
template <typename T>
decltype(auto) deref_optional (optional<T> & opt) noexcept {
	return detail::deref_optional(*opt);
}
template <typename T>
decltype(auto) deref_optional (const optional<T> & opt) noexcept {
	return detail::deref_optional(*opt);
}

template <typename F, typename... Ts>
using bind_optional_t = decltype(
	mcpp::unwrap_optional(
		std::declval<F>()(
			detail::deref_optional(std::declval<Ts>())...
		)
	)
);

template <typename F, typename... Ts>
constexpr bool bind_optional_nothrow = 
	std::is_nothrow_move_constructible<bind_optional_t<F, Ts...>>::value &&
	noexcept(
		mcpp::unwrap_optional(
			std::declval<F>()(
				detail::deref_optional(std::declval<Ts>())...
			)
		)
	);

}

/**
 *	Invokes a function object with the innermost
 *	value of any number of optional objects assuming
 *	all optional objects have a value at their
 *	innermost level. If it is not the case that all
 *	optional objects have a value at their innermost
 *	level an empty optional is returned. Values which
 *	are not optional object may be passed and will be
 *	passed through unchanged.
 *
 *	\tparam F
 *		The function object type to conditionally invoke.
 *	\tparam Ts
 *		The objects to pass through to the function
 *		object.
 *
 *	\param [in] func
 *		The function object to conditionally invoke.
 *	\param [in] args
 *		The arguments to filter and pass through to
 *		\em func assuming all optional objects herein
 *		have a value at their innermost level.
 *
 *	\return
 *		The return value of \em func wrapped in an
 *		optional if \em func is invoked, an optional
 *		which does not return a value otherwise. If
 *		\em func returns an optional the innermost
 *		value of that optional wrapped in an optional
 *		will be returned provided that return value
 *		has a value at its innermost level. If the
 *		return value does not have a value at its
 *		innermest level an optional that does not have
 *		a value is returned.
 */
template <typename F, typename... Ts>
detail::bind_optional_t<F, Ts...> bind_optional (F && func, Ts &&... args) noexcept(detail::bind_optional_nothrow<F, Ts...>) {
	if (!detail::check_optionals(args...)) return nullopt;
	return mcpp::unwrap_optional(std::forward<F>(func)(detail::deref_optional(args)...));
}

}
