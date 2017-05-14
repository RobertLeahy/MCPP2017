#include <mcpp/yggdrasil/user.hpp>
#include <utility>

namespace mcpp {
namespace yggdrasil {

user::user (std::string id, properties_type properties)
	:	id(std::move(id)),
		properties(properties)
{	}

}
}
