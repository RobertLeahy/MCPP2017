#include <mcpp/protocol/exception.hpp>
#include <sstream>
#include <string>

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

}
}
