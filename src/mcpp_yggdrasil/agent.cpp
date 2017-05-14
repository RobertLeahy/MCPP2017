#include <mcpp/yggdrasil/agent.hpp>
#include <string>
#include <utility>

namespace mcpp {
namespace yggdrasil {

agent::agent () : agent(std::string{}, 1) {	}

agent::agent (std::string name, unsigned version)
	:	name(std::move(name)),
		version(version)
{	}

}
}
