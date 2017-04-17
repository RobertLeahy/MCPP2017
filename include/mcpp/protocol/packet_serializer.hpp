/**
 *	\file
 */

#pragma once

#include "../polymorphic_ptr.hpp"
#include "packet.hpp"
#include "packet_id.hpp"
#include <boost/expected/expected.hpp>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <system_error>

namespace mcpp {
namespace protocol {

/**
 *	A base class for implementations which
 *	serialize \ref packet objects to bytes
 *	and which parse \ref packet objects from
 *	bytes.
 *
 *	\tparam Source
 *		A type which models `Source` which shall
 *		be used to read bytes.
 *	\tparam Sink
 *		A type which models `Sink` which shall be
 *		used to write bytes.
 *	\tparam Allocator
 *		A type which models `Allocator` which shall
 *		be used to allocate memory. Defaults to
 *		`std::allocator<packet>`.
 */
template <typename Source, typename Sink, typename Allocator = std::allocator<packet>>
class packet_serializer {
public:
	/**
	 *	A \ref polymorphic_ptr which manages
	 *	\ref packet objects and which uses
	 *	\em Allocator.
	 */
	using pointer = polymorphic_ptr<packet, std::conditional_t<
		std::is_same<Allocator, std::allocator<packet>>::value,
		mcpp::detail::polymorphic_ptr_malloc_tag,
		Allocator
	>>;
	/**
	 *	The `Source` which shall be used for
	 *	read operations.
	 */
	using source_type = Source;
	/**
	 *	The `Sink which shall be used for write
	 *	operations.
	 */
	using sink_type = Sink;
	/**
	 *	@em Allocator.
	 */
	using allocator_type = Allocator;
	/**
	 *	The type returned by \ref parse.
	 */
	using parse_result_type = boost::expected<void, std::error_code>;
	/**
	 *	Creates a packet_serializer object which uses a certain
	 *	allocator.
	 *
	 *	\param [in] a
	 *		The allocator to use. Defaults to a default
	 *		constructed \em Allocator.
	 */
	explicit packet_serializer (const Allocator & a = Allocator{}) noexcept(
		std::is_nothrow_copy_constructible<Allocator>::value
	)	:	a_(a)
	{	}
	packet_serializer (const packet_serializer &) = delete;
	packet_serializer (packet_serializer &&) = delete;
	packet_serializer & operator = (const packet_serializer &) = delete;
	packet_serializer & operator = (packet_serializer &&) = delete;
	/**
	 *	Allows derived classes to be cleaned up through
	 *	pointer or reference to base.
	 */
	virtual ~packet_serializer () noexcept {	}
	/**
	 *	Creates a \ref packet object by consuming characters
	 *	from a `Source`.
	 *
	 *	\param [in] src
	 *		The `Source` from which the representation of
	 *		a \ref packet shall be read.
	 *	\param [out] ptr
	 *		A \em pointer in which the resulting \ref packet
	 *		shall be emplaced. This is an out parameter rather
	 *		than being returned directly from the function as
	 *		it allows the caller to maintain the managed
	 *		pool of memory even in the throwing case.
	 *
	 *	\return
	 *		A `std::error_code` if a \ref packet could not be
	 *		parsed. No result otherwise.
	 */
	virtual parse_result_type parse (Source & src, pointer & ptr) const = 0;
	/**
	 *	Writes the bytes that represent a \ref packet to
	 *	a `Sink`.
	 *
	 *	\param [in] p
	 *		The \ref packet to be written.
	 *	\param [in] sink
	 *		The `Sink` to which the representation of \em p
	 *		shall be written.
	 */
	virtual void serialize (const packet & p, Sink & sink) const = 0;
	/**
	 *	Retrieves a `std::type_info` object representing
	 *	the type derived from \ref packet which this object
	 *	serializes and parses.
	 *
	 *	\return
	 *		A `std::type_info`.
	 */
	virtual const std::type_info & type () const noexcept = 0;
	/**
	 *	Retrieves a \ref packet_id object representing the
	 *	details about when the packet is received and how
	 *	it is identified on the wire.
	 *
	 *	\return
	 *		A \ref packet_id.
	 */
	virtual packet_id id () const noexcept = 0;
	/**
	 *	Retrieves the `Allocator` this object contains.
	 *
	 *	\return
	 *		The contained `Allocator`.
	 */
	const Allocator & get_allocator () const noexcept {
		return a_;
	}
private:
	Allocator a_;
};

/**
 *	Obtains an `Allocator` suitable for allocating
 *	objects of type \em T by possibly rebinding the
 *	`Allocator` managed by a \ref packet_serializer.
 *
 *	This is provided as a free function rather than
 *	as a method of \ref packet_serializer to avoid the need
 *	to use the `template` keyword.
 *
 *	\tparam T
 *		The type of object which the returned
 *		`Allocator` shall allocate.
 *	\tparam Source
 *		The \em Source template parameter of the
 *		\ref packet_serializer which is passed as a parameter.
 *	\tparam Sink
 *		The \em Sink template parameter of the
 *		\ref packet_serializer which is passed as a parameter.
 *	\typename Allocator
 *		The \em Allocator template parameter of the
 *		\ref packet_serializer which is passed as a parameter.
 *		The `Allocator` which is returned by this
 *		function shall be a rebound version of this
 *		`Allocator` equivalent to
 *		`std::allocator_traits<Allocator>::rebind_alloc<T>`.
 *
 *	\param [in] ser
 *		The \ref packet_serializer whose `Allocator` shall be
 *		possibly rebound.
 *
 *	\return
 *		An `Allocator` for allocating objects of type
 *		\em T.
 */
template <typename T, typename Source, typename Sink, typename Allocator>
typename std::allocator_traits<Allocator>::template rebind_alloc<T> get_allocator (const packet_serializer<Source, Sink, Allocator> & ser) noexcept(
	std::is_nothrow_constructible<typename std::allocator_traits<Allocator>::template rebind_alloc<T>, const Allocator &>::value &&
	std::is_nothrow_move_constructible<typename std::allocator_traits<Allocator>::template rebind_alloc<T>>::value
) {
	return typename std::allocator_traits<Allocator>::template rebind_alloc<T>(ser.get_allocator());
}

}
}
