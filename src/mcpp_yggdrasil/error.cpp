#include <beast/core/error.hpp>
#include <mcpp/yggdrasil/error.hpp>
#include <utility>

namespace mcpp {
namespace yggdrasil {

api_error::api_error (
	std::string error,
	std::string error_message,
	optional<std::string> cause
)	:	error(std::move(error)),
		error_message(std::move(error_message)),
		cause(std::move(cause))
{	}

error::error (
	beast::error_code code,
	optional<api_error> api
)	:	beast::error_code(code),
		api(std::move(api))
{	}

}
}
