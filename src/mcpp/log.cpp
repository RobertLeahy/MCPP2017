#include <mcpp/log.hpp>

namespace mcpp {

log::~log () noexcept {	}

void log::write (const std::string & component, std::string message, log_level l) {
	if (!ignored(l)) write_impl(component, message, l);
}

}
