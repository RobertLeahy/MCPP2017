/**
 *	\file
 */

#pragma once

#include "../allocate_unique.hpp"
#include "handshaking.hpp"
#include "packet.hpp"
#include "packet_id.hpp"
#include "packet_parameters.hpp"
#include "packet_serializer.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/tag.hpp>
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <utility>

namespace mcpp {
namespace protocol {

namespace detail {

template <typename Source, typename Sink, typename Allocator>
class packet_id_extractor {
public:
	using result_type = packet_id;
	packet_id operator () (const allocate_unique_t<packet_serializer<Source, Sink, Allocator>, Allocator> & ptr) const noexcept {
		return ptr->id();
	}
};

template <typename Source, typename Sink, typename Allocator>
class type_extractor {
public:
	using result_type = const std::type_info &;
	result_type operator () (const allocate_unique_t<packet_serializer<Source, Sink, Allocator>, Allocator> & ptr) const noexcept {
		return ptr->type();
	}
};

}

/**
 *	The type of a \ref packet_serializer map. Is a
 *	`boost::multi_index::multi_index_container` with certain
 *	template arguments such that it is indexed both by
 *	`std::type_info` and by \ref packet_id. `std::type_info` and
 *	\ref packet_id are tags for their respective indices and
 *	therefore it is not necessary for consumers of this type
 *	to know the index of the indices (and consumers should not
 *	rely on this if they do know it through inspecting the
 *	machinery).
 *
 *	\tparam Source
 *		The `Source` \ref packet_serializer objects contained within
 *		the map shall use to perform read operations.
 *	\tparam Sink
 *		The `Sink` \ref packet_serializer objects contained within the
 *		map shall used to perform write operations.
 *	\tparam Allocator
 *		The type of allocator that shall be used by \ref packet_serializer
 *		objects and by the resulting `boost::multi_index::multi_index_container`.
 *		Defaults to `std::allocator<packet>`.
 */
template <typename Source, typename Sink, typename Allocator = std::allocator<packet>>
using packet_serializer_map_t = boost::multi_index::multi_index_container<
	allocate_unique_t<
		packet_serializer<Source, Sink, Allocator>,
		Allocator
	>,
	boost::multi_index::indexed_by<
		boost::multi_index::hashed_unique<
			boost::multi_index::tag<packet_id>,
			detail::packet_id_extractor<Source, Sink, Allocator>,
			std::hash<packet_id>
		>,
		boost::multi_index::hashed_unique<
			boost::multi_index::tag<std::type_info>,
			detail::type_extractor<Source, Sink, Allocator>
		>
	>,
	Allocator
>;

namespace detail {

template <
	template <typename, typename, typename> class Parameterizedpacket_serializer,
	typename PacketParameters,
	typename Source,
	typename Sink,
	typename Allocator
>
void insert (packet_serializer_map_t<Source, Sink, Allocator> & map) {
	using map_type = packet_serializer_map_t<Source, Sink, Allocator>;
	using pointer = typename map_type::value_type;
	pointer ptr = mcpp::allocate_unique<
		Parameterizedpacket_serializer<
			Source,
			Sink,
			PacketParameters
		>
	>(map.get_allocator());
	map.insert(std::move(ptr));
}

template <typename Index, typename Source, typename Sink, typename Allocator, typename T>
const packet_serializer<Source, Sink, Allocator> * get (const packet_serializer_map_t<Source, Sink, Allocator> & map, const T & val) noexcept {
	auto && i = map.template get<Index>();
	auto iter = i.find(val);
	if (iter == i.end()) return nullptr;
	return iter->get();
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

/**
 *	Attempts to locate a \ref packet_serializer which parses and serializes
 *	packets of a certain type.
 *
 *	\tparam Source
 *		The \em Source parameter to the \ref packet_serializer_map_t to search.
 *	\tparam Sink
 *		The \em Sink parameter to the \ref packet_serializer_map_t to search.
 *	\tparam Allocator
 *		The \em Allocator parameter to the \ref packet_serializer_map_t to search.
 *
 *	\param [in] map
 *		A \ref packet_serializer_map_t which shall be searched.
 *	\param [in] type
 *		A `std::type_info` representing the type of \ref packet for which
 *		a packet_serializer shall be found.
 *
 *	\return
 *		A pointer to an appropriate \ref packet_serializer if one is found.
 *		`nullptr` otherwise.
 */
template <typename Source, typename Sink, typename Allocator>
const packet_serializer<Source, Sink, Allocator> * get (const packet_serializer_map_t<Source, Sink, Allocator> & map, const std::type_info & type) noexcept {
	return detail::get<std::type_info>(map, type);
}
/**
 *	Attempts to locate a \ref packet_serializer which parses and serializes
 *	a certain \ref packet.
 *
 *	\tparam Source
 *		The \em Source parameter to the \ref packet_serializer_map_t to search.
 *	\tparam Sink
 *		The \em Sink parameter to the \ref packet_serializer_map_t to search.
 *	\tparam Allocator
 *		The \em Allocator parameter to the \ref packet_serializer_map_t to search.
 *
 *	\param [in] map
 *		A \ref packet_serializer_map_t which shall be searched.
 *	\param [in] p
 *		The \ref packet which is to be parsed or serialized.
 *
 *	\return
 *		A pointer to an appropriate \ref packet_serializer if one is found.
 *		`nullptr` otherwise.
 */
template <typename Source, typename Sink, typename Allocator>
const packet_serializer<Source, Sink, Allocator> * get (const packet_serializer_map_t<Source, Sink, Allocator> & map, const packet & p) noexcept {
	return protocol::get(map, typeid(p));
}
/**
 *	Attempts to locate a \ref packet_serializer which parses and serializes
 *	a type of \ref packet identified by a \ref packet_id object.
 *
 *	\tparam Source
 *		The \em Source parameter to the \ref packet_serializer_map_t to search.
 *	\tparam Sink
 *		The \em Sink parameter to the \ref packet_serializer_map_t to search.
 *	\tparam Allocator
 *		The \em Allocator parameter to the \ref packet_serializer_map_t to search.
 *
 *	\param [in] map
 *		A \ref packet_serializer_map_t which shall be searched.
 *	\param [in] id
 *		The \ref packet_id for the type of \ref packet which is to be
 *		parsed or serialized.
 *
 *	\return
 *		A pointer to an appropriate \ref packet_serializer if one is found.
 *		`nullptr` otherwise.
 */
template <typename Source, typename Sink, typename Allocator>
const packet_serializer<Source, Sink, Allocator> * get (const packet_serializer_map_t<Source, Sink, Allocator> & map, const packet_id & id) noexcept {
	return detail::get<packet_id>(map, id);
}

}
}
