#include <mcpp/yggdrasil/refresh.hpp>
#include <string>
#include <utility>

namespace mcpp {
namespace yggdrasil {

refresh_request::refresh_request () : refresh_request(std::string{}, std::string{}) {	}

refresh_request::refresh_request (
	std::string access_token,
	std::string client_token,
	optional<profile> selected_profile,
	bool request_user
)	:	access_token(std::move(access_token)),
		client_token(std::move(client_token)),
		selected_profile(std::move(selected_profile)),
		request_user(request_user)
{	}

refresh_response::refresh_response (
	std::string access_token,
	std::string client_token,
	optional<profile> selected_profile,
	optional<yggdrasil::user> user
)	:	access_token(std::move(access_token)),
		client_token(std::move(client_token)),
		selected_profile(std::move(selected_profile)),
		user(std::move(user))
{	}

}
}
