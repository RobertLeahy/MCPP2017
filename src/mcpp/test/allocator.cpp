#include <mcpp/test/allocator.hpp>

namespace mcpp {
namespace test {

allocator_state::allocator_state () noexcept
	:	allocations(0),
		failed_allocations(0),
		deallocations(0),
		allocated(0),
		deallocated(0),
		rebound(0)
{	}

}
}
