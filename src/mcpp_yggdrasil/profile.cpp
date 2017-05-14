#include <mcpp/yggdrasil/profile.hpp>
#include <string>
#include <utility>

namespace mcpp {
namespace yggdrasil {

profile::profile () : profile(std::string{}, std::string{}) {	}

profile::profile (std::string id, std::string name, bool legacy)
	:	id(std::move(id)),
		name(std::move(name)),
		legacy(legacy)
{	}

}
}
