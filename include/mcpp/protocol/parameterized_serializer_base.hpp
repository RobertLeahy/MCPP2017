/**
 *	\file
 */

#pragma once

#include "direction.hpp"
#include "packet_parameters.hpp"
#include "serializer_base.hpp"
#include "state.hpp"
#include <cstdint>

namespace mcpp {
namespace protocol {

/**
 *	Derives from \ref serializer_base. Rather than dealing
 *	in an absolute type which derives from \ref packet this
 *	class deals with a type templates on a parameter which
 *	models `PacketParameters`.
 *
 *	\tparam ParameterizedPacket
 *		A template of one parameter which when instantiated
 *		with a model of `PacketParameters` creates a type which
 *		derives from \ref packet.
 *	\tparam Id
 *		See \ref serializer_base.
 *	\tparam Direction
 *		See \ref serializer_base.
 *	\tparam State
 *		See \ref serializer_base.
 *	\tparam Source
 *		See \ref serializer_base.
 *	\tparam Sink
 *		See \ref serializer_base.
 *	\tparam PacketParameters
 *		A type which models `PacketParameters` which shall be used
 *		to obtain a concrete type from \em ParameterizedPacket and
 *		from which a model of `Allocator` shall be extracted to
 *		pass through to the appropriate template parameter of
 *		\ref serializer_base. Defaults to \ref packet_parameters.
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
class parameterized_serializer_base : public serializer_base<
	ParameterizedPacket<PacketParameters>,
	Id,
	Direction,
	State,
	Source,
	Sink,
	allocator_t<PacketParameters>
> {
private:
	using base = serializer_base<
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
