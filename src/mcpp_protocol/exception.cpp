#include <mcpp/protocol/exception.hpp>
#include <sstream>
#include <string>
#include <typeinfo>

namespace mcpp {
namespace protocol {

static std::string get_write_overflow_error (std::size_t attempted, std::size_t actual) {
	std::ostringstream ss;
	ss << "Stream refused write of " << attempted << " bytes (" << actual << " written)";
	return ss.str();
}

write_overflow_error::write_overflow_error (std::size_t attempted, std::size_t actual)
	:	write_error(get_write_overflow_error(attempted, actual))
{	}

static std::string packet_serializer_not_found_what (const std::type_info &) {
	//	TODO: Actually stringify the type
	return "Packet serializer not found";
}

packet_serializer_not_found::packet_serializer_not_found (const packet & p)
	:	packet_serializer_not_found(typeid(p))
{	}

packet_serializer_not_found::packet_serializer_not_found (const std::type_info & type)
	:	serialize_error(packet_serializer_not_found_what(type)),
		type_(&type)
{	}

const std::type_info & packet_serializer_not_found::type () const noexcept {
	return *type_;
}

}
}
