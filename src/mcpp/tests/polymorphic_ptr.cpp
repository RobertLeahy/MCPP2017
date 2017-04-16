#include <mcpp/polymorphic_ptr.hpp>
#include <mcpp/optional.hpp>
#include <mcpp/test/allocator.hpp>
#include <mcpp/test/object.hpp>
#include <cstddef>
#include <typeinfo>
#include <catch.hpp>

namespace mcpp {
namespace tests {
namespace {

class derived : public test::object {
public:
	using test::object::object;
	derived () = delete;
	derived (test::object::state & state, bool & destroyed)
		:	test::object(state),
			destroyed_(destroyed)
	{}
	~derived () noexcept {
		destroyed_ = true;
	}
private:
	bool & destroyed_;
	//	Forces an allocation where the polymorphic_ptr
	//	allocation policy of allocating with a std::max_align_t
	//	allocator (to ensure suitable alignedness for any type)
	//	might make a reallocation unnecessary
	char arr [sizeof(std::max_align_t)];
};
class vbase : public test::object {
public:
	using test::object::object;
	vbase () = default;
	vbase (const vbase &) = delete;
	vbase (vbase &&) = delete;
	vbase & operator = (const vbase &) = delete;
	vbase & operator = (vbase &&) = delete;
	virtual ~vbase () noexcept {	}
};
class vderived : public vbase {
public:
	vderived () = delete;
	vderived (test::object::state & state, bool & destroyed)
		:	vbase(state),
			destroyed_(destroyed)
	{	}
	~vderived () noexcept {
		destroyed_ = true;
	}
private:
	bool & destroyed_;
};

SCENARIO("mcpp::polymorphic_ptr manages the lifetime of objects with a common base class", "[mcpp][polymorphic_ptr]") {
	GIVEN("An mcpp::polymorphic_ptr which wraps a type which lacks a virtual destructor") {
		test::object::state state;
		bool destroyed = false;
		polymorphic_ptr<test::object> ptr;
		THEN("It does not manage an object") {
			CHECK_FALSE(ptr);
		}
		THEN("It manages no memory") {
			CHECK(ptr.capacity() == 0);
		}
		WHEN("An object of derived type is emplaced therein") {
			ptr.emplace<derived>(state, destroyed);
			THEN("It manages memory") {
				CHECK(ptr.capacity() == sizeof(derived));
			}
			THEN("It manages an object") {
				CHECK(ptr);
			}
			THEN("Its type cannot be detected via RTTI") {
				CHECK(typeid(*ptr) == typeid(test::object));
			}
			AND_WHEN("The object is destroyed") {
				ptr.reset();
				THEN("It no longer manages an object") {
					CHECK_FALSE(ptr);
				}
				THEN("It still manages memory") {
					CHECK(ptr.capacity() == sizeof(derived));
				}
				THEN("The derived class is destroyed") {
					CHECK(destroyed);
				}
				THEN("The base class is destroyed") {
					CHECK(state.destruct == 1);
				}
			}
		}
	}
	GIVEN("An mcpp::polymorphic_ptr which wraps a type which has a virtual destructor") {
		test::object::state state;
		bool destroyed = false;
		polymorphic_ptr<vbase> ptr;
		THEN("It does not manage an object") {
			CHECK_FALSE(ptr);
		}
		THEN("It manages no memory") {
			CHECK(ptr.capacity() == 0);
		}
		WHEN("An object of derived type is emplaced therein") {
			ptr.emplace<vderived>(state, destroyed);
			THEN("It manages memory") {
				CHECK(ptr.capacity() == sizeof(vderived));
			}
			THEN("It manages an object") {
				CHECK(ptr);
			}
			THEN("Its type can be detected via RTTI") {
				CHECK(typeid(*ptr) == typeid(vderived));
			}
			AND_WHEN("The object is destroyed") {
				ptr.reset();
				THEN("It no longer manages an object") {
					CHECK_FALSE(ptr);
				}
				THEN("It still manages memory") {
					CHECK(ptr.capacity() == sizeof(vderived));
				}
				THEN("The derived class is destroyed") {
					CHECK(destroyed);
				}
				THEN("The base class is destroyed") {
					CHECK(state.destruct == 1);
				}
			}
		}
	}
}

SCENARIO("mcpp::polymorphic_ptr may use custom allocators to obtain memory", "[mcpp][polymorphic_ptr]") {
	GIVEN("An mcpp::polymorphic_ptr with a custom allocator type") {
		test::allocator_state state;
		test::allocator<test::object> a(state);
		test::object::state ostate;
		bool destroyed = false;
		optional<polymorphic_ptr<test::object, decltype(a)>> ptr(in_place, a);
		THEN("It does not manage an object") {
			CHECK_FALSE(*ptr);
		}
		THEN("It does not manage memory") {
			CHECK(ptr->capacity() == 0);
		}
		THEN("It does not perform any allocations") {
			CHECK(state.allocations == 0);
		}
		WHEN("Its lifetime ends") {
			ptr = nullopt;
			THEN("It does not deallocate any memory") {
				CHECK(state.deallocated == 0);
			}
		}
		WHEN("An object is emplaced therein") {
			ptr->emplace<test::object>(ostate);
			THEN("It manages an object") {
				CHECK(*ptr);
			}
			THEN("It manages memory") {
				CHECK(ptr->capacity() >= sizeof(test::object));
			}
			auto prev_cap = ptr->capacity();
			THEN("It performs an allocation") {
				CHECK(state.allocations == 1);
			}
			THEN("The correct amount of memory is allocated") {
				CHECK(state.allocated >= sizeof(test::object));
			}
			AND_WHEN("The managed object is destroyed") {
				ptr->reset();
				THEN("It does not manage an object") {
					CHECK_FALSE(*ptr);
				}
				THEN("It manages memory") {
					CHECK(ptr->capacity() >= sizeof(test::object));
					CHECK(state.allocated == ptr->capacity());
				}
				THEN("No deallocation is performed") {
					CHECK(state.deallocations == 0);
				}
				THEN("No allocation is performed") {
					CHECK(state.allocations == 1);
				}
			}
			AND_WHEN("A larger object is emplaced therein") {
				ptr->emplace<derived>(ostate, destroyed);
				THEN("The previously managed object is destroyed") {
					CHECK(ostate.destruct == 1);
				}
				THEN("It manages an object") {
					CHECK(*ptr);
				}
				THEN("It manages more memory") {
					CHECK(ptr->capacity() >= sizeof(derived));
					CHECK(ptr->capacity() > prev_cap);
					CHECK(state.allocated == (prev_cap + ptr->capacity()));
				}
				THEN("A dellocation is performed") {
					CHECK(state.deallocations == 1);
					CHECK(state.deallocated == prev_cap);
				}
				prev_cap = ptr->capacity();
				THEN("An allocation is performed") {
					CHECK(state.allocations == 2);
				}
				AND_WHEN("A smaller object is again emplaced therein") {
					ptr->emplace<test::object>(ostate);
					THEN("The previously managed object is destroyed") {
						CHECK(ostate.destruct == 2);
					}
					THEN("It manages an object") {
						CHECK(*ptr);
					}
					THEN("It manages the same amount of memory") {
						CHECK(ptr->capacity() == prev_cap);
					}
					THEN("No deallocation is performed") {
						CHECK(state.deallocations == 1);
					}
					THEN("No allocation is performed") {
						CHECK(state.allocations == 2);
					}
				}
			}
			AND_WHEN("The lifetime of the mcpp::polymorphic_ptr ends") {
				ptr = nullopt;
				THEN("The managed object's lifetime ends") {
					CHECK(ostate.destruct == 1);
				}
				THEN("The managed memory is deallocated") {
					CHECK(state.deallocations == 1);
					CHECK(state.deallocated == state.allocated);
				}
			}
		}
	}
}

}
}
}
