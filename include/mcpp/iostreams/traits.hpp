/**
 *	\file
 */

#pragma once

#include <boost/iostreams/char_traits.hpp>
#include <boost/iostreams/traits.hpp>
#include <type_traits>

namespace mcpp {
namespace iostreams {

/**
 *	Resolves to `typename boost::iostreams::char_type_of<T>::type`.
 *
 *	\tparam T
 *		See above.
 */
template <typename T>
using char_type_of_t = typename boost::iostreams::char_type_of<T>::type;

/**
 *	Resolves to `typename boost::iostreams::int_type_of<T>::type`.
 *
 *	\tparam T
 *		See above.
 */
template <typename T>
using int_type_of_t = typename boost::iostreams::int_type_of<T>::type;

/**
 *	Resolves to `typename boost::iostreams::category_of<T>::type`.
 *
 *	\tparam T
 *		See above.
 */
template <typename T>
using category_of_t = typename boost::iostreams::category_of<T>::type;

/**
 *	Resolves to `boost::iostreams::char_traits<char_type_of_t<T>>`.
 *
 *	\tparam T
 *		See above.
 */
template <typename T>
using traits_of_t = boost::iostreams::char_traits<char_type_of_t<T>>;

/**
 *	\em true if a certain type models a certain category
 *	from Boost.IOStreams, \em false otherwise.
 *
 *	\tparam T
 *		The type to test.
 *	\tparam Category
 *		The category to check for.
 */
template <typename T, typename Category>
constexpr bool in_category_v = std::is_convertible<category_of_t<T>, Category>::value;

/**
 *	`std::true_type` if @ref in_category_v is \em true,
 *	`std::false_type` otherwise.
 *
 *	\tparam T
 *		The type to test.
 *	\tparam Category
 *		The category to check for.
 */
template <typename T, typename Category>
using in_category_t = std::integral_constant<bool, in_category_v<T, Category>>;

namespace detail {

template <typename, typename...>
class in_any_category;
template <typename T>
class in_any_category<T> {
public:
	static constexpr bool value = false;
};
template <typename T, typename Category, typename... Categories>
class in_any_category<T, Category, Categories...> {
public:
	static constexpr bool value = in_category_v<T, Category> ||
		in_any_category<T, Categories...>::value;
};

}

/**
 *	\em true if a certain type models any of a set
 *	of categories, \em false otherwise.
 *
 *	If the empty set of categories is provided is
 *	\em false.
 *
 *	\tparam T
 *		The type to test.
 *	\tparam Categories
 *		The categories to check.
 */
template <typename T, typename... Categories>
constexpr bool in_any_category_v = detail::in_any_category<T, Categories...>::value;

/**
 *	`std::true_type` if @ref in_any_category_v is \em true,
 *	`std::false_type` otherwise.
 *
 *	\tparam T
 *		The type to test.
 *	\tparam Categories
 *		The categories to check.
 */
template <typename T, typename... Categories>
using in_any_category_t = std::integral_constant<bool, in_any_category_v<T, Categories...>>;

}
}
