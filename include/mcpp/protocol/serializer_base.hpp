/**
 *	\file
 */

#pragma once

#include "direction.hpp"
#include "packet_id.hpp"
#include "serializer.hpp"
#include "state.hpp"
#include <cassert>
#include <cstdint>
#include <memory>
#include <typeinfo>
#include <utility>

namespace mcpp {
namespace protocol {

/**
 *	Derives from \ref serializer and provides
 *	implementations of \ref serializer::serialize,
 *	\ref serializer::type, and \ref serializer::id
 *	as appropriate based on the template parameters.
 *
 *	\tparam Packet
 *		The type of \ref packet the derived type is
 *		responsible for serializing and parsing. The
 *		second parameter to \ref serializer::serialize
 *		will be appropriately downcast before being
 *		passed through to a virtual method implemented
 *		by the derived class and introduced by this
 *		class as per the template method pattern.
 *	\tparam Id
 *		An integer giving the numeric ID of the
 *		appropriate \ref packet as represented on the
 *		wire.
 *	\tparam Direction
 *		A \ref direction value giving the direction in
 *		which the appropriate \ref packet is sent.
 *	\tparam State
 *		A \ref state value giving the state in which the
 *		appropriate \ref packet is sent and received.
 *	\tparam Source
 *		Passed through to \ref serializer.
 *	\tparam Sink
 *		Passed through to \ref serializer.
 *	\tparam Allocator
 *		A model of `Allocator` to pass through to
 *		\serializer. Defaults to `std::allocator<packet>`
 *		just as in the case of \ref serializer.
 */
template <
	typename Packet,
	std::uint32_t Id,
	direction Direction,
	state State,
	typename Source,
	typename Sink,
	typename Allocator
>
class serializer_base : public serializer<Source, Sink, Allocator> {
private:
	using base = serializer<Source, Sink, Allocator>;
public:
	using base::base;
	/**
	 *	The class which derives from \ref packet which
	 *	objects of this type parse and serialize.
	 */
	using packet_type = Packet;
	virtual void serialize (const packet & p, Sink & sink) const final override {
		assert(dynamic_cast<const Packet *>(&p));
		serialize(static_cast<const Packet &>(p), sink);
	}
	/**
	 *	Performs the same task as \ref serializer::serialize
	 *	except that the second parameter has type `const Packet &`
	 *	rather than `const packet &` (i.e. this base class
	 *	performs the appropriate down cast for you and invokes
	 *	this method as per the template method pattern).
	 *
	 *	\param [in] p
	 *		The \ref packet to write.
	 *	\param [in] sink
	 *		The `Sink` to which the representation of \em p
	 *		shall be written.
	 */
	virtual void serialize (const Packet & p, Sink & sink) const = 0;
	virtual const std::type_info & type () const noexcept final override {
		return typeid(Packet);
	}
	virtual packet_id id () const noexcept final override {
		return packet_id(Id, Direction, State);
	}
};

}
}
