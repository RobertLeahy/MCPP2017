#include <mcpp/test/object.hpp>
#include <exception>

namespace mcpp {
namespace test {

object::state::state () noexcept
	:	construct(0),
		copy_construct(0),
		move_construct(0),
		copy_assign(0),
		move_assign(0),
		destruct(0)
{}

static void throw_if_applicable (const std::exception_ptr & ex) {
	if (ex) std::rethrow_exception(ex);
}

object::object (const object & other) : state_(other.state_) {
	++state_->copy_construct;
	throw_if_applicable(state_->copy_construct_exception);
}

object::object (object && other) : state_(other.state_) {
	++state_->move_construct;
	throw_if_applicable(state_->move_construct_exception);
}

object & object::operator = (const object & rhs) {
	++state_->copy_assign;
	state_ = rhs.state_;
	throw_if_applicable(state_->copy_assign_exception);
	return *this;
}

object & object::operator = (object && rhs) {
	++state_->move_assign;
	state_ = rhs.state_;
	throw_if_applicable(state_->move_assign_exception);
	return *this;
}

object::object (state & s) : state_(&s) {
	++s.construct;
	throw_if_applicable(s.construct_exception);
}

object::~object () noexcept {
	++state_->destruct;
}

}
}
