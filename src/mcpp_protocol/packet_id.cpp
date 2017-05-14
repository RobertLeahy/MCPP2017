#include <boost/functional/hash.hpp>
#include <mcpp/protocol/direction.hpp>
#include <mcpp/protocol/packet_id.hpp>
#include <mcpp/protocol/state.hpp>
#include <type_traits>

namespace mcpp {
namespace protocol {

packet_id::packet_id (id_type id, protocol::direction d, protocol::state s) noexcept
	:	id_(id),
		d_(d),
		s_(s)
{	}

packet_id::id_type packet_id::id () const noexcept {
	return id_;
}

direction packet_id::direction () const noexcept {
	return d_;
}

state packet_id::state () const noexcept {
	return s_;
}

bool operator == (const packet_id & lhs, const packet_id & rhs) noexcept {
	return (lhs.id() == rhs.id()) && (lhs.direction() == rhs.direction()) && (lhs.state() == rhs.state());
}

bool operator != (const packet_id & lhs, const packet_id & rhs) noexcept {
	return !(lhs == rhs);
}

bool operator < (const packet_id & lhs, const packet_id & rhs) noexcept {
	using s_type = std::underlying_type_t<state>;
	auto ls = static_cast<s_type>(lhs.state());
	auto rs = static_cast<s_type>(rhs.state());
	if (ls != rs) {
		return ls < rs;
	}
	using d_type = std::underlying_type_t<direction>;
	auto ld = static_cast<d_type>(lhs.direction());
	auto rd = static_cast<d_type>(rhs.direction());
	if (ld != rd) {
		return ld < rd;
	}
	return lhs.id() < rhs.id();
}

}
}

namespace std {

std::hash<mcpp::protocol::packet_id>::result_type std::hash<mcpp::protocol::packet_id>::operator () (argument_type id) const noexcept {
	result_type seed = 0;
	boost::hash_combine(seed, id.id());
	boost::hash_combine(seed, id.direction());
	boost::hash_combine(seed, id.state());
	return seed;
}

}
