#include <mcpp/yggdrasil/signout.hpp>
#include <utility>

namespace mcpp {
namespace yggdrasil {

signout_request::signout_request (std::string username, std::string password)
	:	username(std::move(username)),
		password(std::move(password))
{	}

}
}
