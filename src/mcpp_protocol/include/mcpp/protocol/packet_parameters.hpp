/**
 *	\file
 */

#pragma once

#include "packet.hpp"
#include <string>
#include <memory>
#include <type_traits>

namespace mcpp {
namespace protocol {

/**
 *	An archetype of the `PacketParameters` concept.
 *
 *	This class serves to deliver type parameters to
 *	classes derived from \ref packet.
 *
 *	The intention is that rather than configuring
 *	individual packet types based on their particular
 *	needs one provides a model of `PacketParameters`
 *	which simultaneously provides all type parameters
 *	needed to parameterize the entire vanilla Minecraft
 *	protocol.
 *
 *	This allows packets to be updated to depend on new
 *	parameters without breaking code that consumes them
 *	(since that code doesn't have to be modified to provide
 *	new template parameters as all that information is
 *	seamlessly carried in a type that models
 *	`PacketParameters`).
 *
 *	When providing a model of `PacketParameters` one should
 *	use this class as a base class. In this way when and if
 *	new requirements are added to the `PacketParameters`
 *	concept the custom `PacketParameters` will automatically
 *	fulfill them.
 */
class packet_parameters {
public:
	/**
	 *	The character type to be used by specializations of
	 *	`std::basic_string`.
	 */
	using char_type = char;
	/**
	 *	The traits type to be used by specializations of
	 *	`std::basic_string`.
	 */
	using traits = std::char_traits<char_type>;
	/**
	 *	The `Allocator` which shall be used to obtain memory
	 *	for collections.
	 */
	using allocator_type = std::allocator<packet>;
};

/**
 *	Extracts `char_type` from a model of `PacketParameters`.
 *
 *	\tparam PacketParameters
 *		A model of `PacketParameters`.
 */
template <typename PacketParameters>
using char_t = typename PacketParameters::char_type;

/**
 *	Extracts `traits` from a model of `PacketParameters`.
 *
 *	\tparam PacketParameters
 *		A model of `PacketParameters`.
 */
template <typename PacketParameters>
using traits_t = typename PacketParameters::traits;

namespace detail {

template <typename PacketParameters>
using allocator_t = typename PacketParameters::allocator_type;

}

/**
 *	Extracts an optionally rebound `allocator_type` from a
 *	model of `PacketParameters`.
 *
 *	\tparam PacketParameters
 *		A model of `PacketParameters`.
 *	\tparam T
 *		The type which the resulting type shall allocate.
 *		Defaults to `PacketParameters::allocator_type::value_type`
 *		in which case the resulting type shall not be rebound.
 */
template <
	typename PacketParameters,
	typename T = typename std::allocator_traits<detail::allocator_t<PacketParameters>>::value_type
>
using allocator_t = typename std::allocator_traits<detail::allocator_t<PacketParameters>>::template rebind_alloc<T>;

/**
 *	Extracts `allocator_type` from a model of `PacketParameters`
 *	rebound to allocate `char_type` extracted from that
 *	same module of `PacketParameters`.
 *
 *	\tparam PacketParameters
 *		A model of `PacketParameters`.
 */
template <typename PacketParameters>
using string_allocator_t = allocator_t<PacketParameters, char_t<PacketParameters>>;

/**
 *	Synthesizes a specialization of `std::basic_string` from
 *	a model of `PacketParameters`.
 *
 *	\tparam PacketParameters
 *		A model of `PacketParameters`.
 */
template <typename PacketParameters>
using string_t = std::basic_string<
	char_t<PacketParameters>,
	traits_t<PacketParameters>,
	string_allocator_t<PacketParameters>
>;

/**
 *	Default constructs a `std::basic_string` appropriately
 *	specialized for a model of `PacketParameters` and which
 *	uses the correct `Allocator`.
 *
 *	\tparam PacketParameters
 *		A model of `PacketParameters`.
 *
 *	\param [in] a
 *		An `Allocator` of type `PacketParameters::allocator_type`.
 *		Defaults to a default constructed `PacketParameters::allocator_type`.
 *
 *	\return
 *		An appropriately constructed and specialized
 *		`std::basic_string`.
 */
template <typename PacketParameters>
string_t<PacketParameters> create_string (const allocator_t<PacketParameters> & a = allocator_t<PacketParameters>{}) noexcept(
	std::is_nothrow_move_constructible<string_t<PacketParameters>>::value
) {
	string_allocator_t<PacketParameters> b(a);
	return string_t<PacketParameters>(b);
}

}
}
