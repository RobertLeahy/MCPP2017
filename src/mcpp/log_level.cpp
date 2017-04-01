#include <mcpp/log_level.hpp>
#include <stdexcept>
#include <string>

namespace mcpp {

const std::string & to_string (log_level l) {
	static const std::string emergency("EMERGENCY");
	static const std::string alert("ALERT");
	static const std::string critical("CRITICAL");
	static const std::string error("ERROR");
	static const std::string warning("WARNING");
	static const std::string notice("NOTICE");
	static const std::string info("INFO");
	static const std::string debug("DEBUG");
	switch (l) {
	case log_level::emergency:
		return emergency;
	case log_level::alert:
		return alert;
	case log_level::critical:
		return critical;
	case log_level::error:
		return error;
	case log_level::warning:
		return warning;
	case log_level::notice:
		return notice;
	case log_level::info:
		return info;
	case log_level::debug:
		return debug;
	default:
		break;
	}
	throw std::logic_error("Unknown log level");
}

}
