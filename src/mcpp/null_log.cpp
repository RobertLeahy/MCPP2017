#include <mcpp/null_log.hpp>

namespace mcpp {

void null_log::write_impl (const std::string &, std::string, level) {	}

bool null_log::ignored (level) {
	return true;
}

}
