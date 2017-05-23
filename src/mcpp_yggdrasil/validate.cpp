#include <mcpp/yggdrasil/validate.hpp>
#include <utility>

namespace mcpp {
namespace yggdrasil {

validate_request::validate_request (std::string access_token, optional<std::string> client_token)
	:	access_token(std::move(access_token)),
		client_token(std::move(client_token))
{	}

}
}
