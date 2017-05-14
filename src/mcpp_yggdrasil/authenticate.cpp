#include <mcpp/yggdrasil/authenticate.hpp>
#include <string>
#include <utility>

namespace mcpp {
namespace yggdrasil {

authenticate_request::authenticate_request ()
	:	authenticate_request(std::string{}, std::string{})
{	}

authenticate_request::authenticate_request (
	std::string username,
	std::string password,
	optional<yggdrasil::agent> agent,
	optional<std::string> client_token,
	bool request_user
)	:	agent(std::move(agent)),
		username(std::move(username)),
		password(std::move(password)),
		client_token(std::move(client_token)),
		request_user(std::move(request_user))
{	}

authenticate_response::authenticate_response (
	std::string access_token,
	std::string client_token,
	optional<available_profiles_type> available_profiles,
	optional<profile> selected_profile,
	optional<yggdrasil::user> user
)	:	access_token(std::move(access_token)),
		client_token(std::move(client_token)),
		available_profiles(std::move(available_profiles)),
		selected_profile(std::move(selected_profile)),
		user(std::move(user))
{	}

}
}
