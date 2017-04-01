#include <mcpp/stream_log.hpp>

namespace mcpp {

void stream_log::write_impl (const std::string & component, std::string message, log_level l) {
	os_ << '[' << to_string(l) << "] [" << component << "] " << message << '\n';
}

stream_log::stream_log (std::ostream & os) noexcept : os_(os) {	}

bool stream_log::ignored (log_level l) {
	return ignored_.count(l);
}

void stream_log::ignore (log_level l) {
	ignored_.insert(l);
}

}
