#include <mcpp/yggdrasil/invalidate.hpp>
#include <utility>

namespace mcpp {
namespace yggdrasil {

invalidate_request::invalidate_request (std::string access_token, std::string client_token)
	:	access_token(std::move(access_token)),
		client_token(std::move(client_token))
{	}

}
}
