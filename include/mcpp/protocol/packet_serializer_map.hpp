/**
 *	\file
 */

#pragma once

#include "../allocate_unique.hpp"
#include "handshaking.hpp"
#include "packet_parameters.hpp"
#include "packet_serializer_map_t.hpp"
#include <utility>

namespace mcpp {
namespace protocol {

namespace detail {

template <
	template <typename, typename, typename> class ParameterizedPacketSerializer,
	typename PacketParameters,
	typename Source,
	typename Sink,
	typename Allocator
>
void insert (packet_serializer_map_t<Source, Sink, Allocator> & map) {
	using map_type = packet_serializer_map_t<Source, Sink, Allocator>;
	using pointer = typename map_type::value_type;
	pointer ptr = mcpp::allocate_unique<
		ParameterizedPacketSerializer<
			Source,
			Sink,
			PacketParameters
		>
	>(map.get_allocator());
	map.insert(std::move(ptr));
}

}

/**
 *	Obtains a \ref packet_serializer_map_t which is populated with
 *	default \ref packet_serializer objects such that the resulting map
 *	is suitable to use to serialize and parse the vanilla
 *	Minecraft protocol.
 *
 *	\tparam Source
 *		The `Source` \ref packet_serializer objects contained within
 *		the map shall use to perform read operations.
 *	\tparam Sink
 *		The `Sink` \ref packet_serializer objects contained within the
 *		map shall used to perform write operations.
 *	\tparam PacketParameters
 *		A model of `PacketParameters` which shall be used to
 *		configure the resulting packets. Defaults to
 *		\ref packet_parameters.
 *
 *	\param [in] a
 *		The `Allocator` to use for all allocations both within
 *		this function and in all resulting \ref packet_serializer objects.
 *		Defaults to a default constructed object of type
 *		`PacketParameters::allocator_type`.
 *
 *	\return
 *		A \ref packet_serializer_map_t.
 */
template <typename Source, typename Sink, typename PacketParameters = packet_parameters>
packet_serializer_map_t<Source, Sink, allocator_t<PacketParameters>> packet_serializer_map (const allocator_t<PacketParameters> & a = allocator_t<PacketParameters>{}) {
	packet_serializer_map_t<Source, Sink, allocator_t<PacketParameters>> retr(a);
	//	Handshaking
	//		Serverbound
	detail::insert<handshaking::serverbound::handshake_serializer, PacketParameters>(retr);
	return retr;
}

}
}
