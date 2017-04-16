/**
 *	\file
 */

#pragma once

#include "direction.hpp"
#include "packet_parameters.hpp"
#include "typed_packet_serializer.hpp"
#include "state.hpp"
#include <cstdint>

namespace mcpp {
namespace protocol {

/**
 *	Derives from \ref typed_serializer. Rather than dealing
 *	in an absolute type which derives from \ref packet this
 *	class deals with a type templates on a parameter which
 *	models `PacketParameters`.
 *
 *	\tparam ParameterizedPacket
 *		A template of one parameter which when instantiated
 *		with a model of `PacketParameters` creates a type which
 *		derives from \ref packet.
 *	\tparam Id
 *		See \ref typed_serializer.
 *	\tparam Direction
 *		See \ref typed_serializer.
 *	\tparam State
 *		See \ref typed_serializer.
 *	\tparam Source
 *		See \ref typed_serializer.
 *	\tparam Sink
 *		See \ref typed_serializer.
 *	\tparam PacketParameters
 *		A type which models `PacketParameters` which shall be used
 *		to obtain a concrete type from \em ParameterizedPacket and
 *		from which a model of `Allocator` shall be extracted to
 *		pass through to the appropriate template parameter of
 *		\ref typed_serializer. Defaults to \ref packet_parameters.
 */
template <
	template <typename> class ParameterizedPacket,
	std::uint32_t Id,
	direction Direction,
	state State,
	typename Source,
	typename Sink,
	typename PacketParameters = packet_parameters
>
class parameterized_packet_serializer : public typed_packet_serializer<
	ParameterizedPacket<PacketParameters>,
	Id,
	Direction,
	State,
	Source,
	Sink,
	allocator_t<PacketParameters>
> {
private:
	using base = typed_packet_serializer<
		ParameterizedPacket<PacketParameters>,
		Id,
		Direction,
		State,
		Source,
		Sink,
		allocator_t<PacketParameters>
	>;
public:
	using base::base;
	/**
	 *	The model of `PacketParameters` on which this type
	 *	was templated.
	 */
	using parameters = PacketParameters;
};


}
}
