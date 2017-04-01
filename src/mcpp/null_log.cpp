#include <mcpp/null_log.hpp>

namespace mcpp {

void null_log::write_impl (const std::string &, std::string, log_level) {	}

bool null_log::ignored (log_level) {
	return true;
}

}
