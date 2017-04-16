/**
 *	\file
 */

#pragma once

#include <cstddef>
#include <exception>

namespace mcpp {
namespace test {

/**
 *	An object which is `CopyConstructible`,
 *	`MoveConstructible`, `CopyAssignable`,
 *	`MoveAssignable`, and `Destructible` and
 *	which instruments each of these operations
 *	counting them in a \ref object::state object.
 */
class object {
public:
	/**
	 *	Records the number of times an \ref object
	 *	is copied, moved, or destroyed.
	 */
	class state {
	public:
		/**
		 *	The number of times the associated \ref object
		 *	objects have had their constructor which accepts
		 *	an object of this type invoked.
		 */
		std::size_t construct;
		/**
		 *	The number of times the associated \ref object
		 *	objects have had their copy constructor invoked.
		 *
		 *	Note that in the case where a copy constructor
		 *	throws it is still counted.
		 */
		std::size_t copy_construct;
		/**
		 *	The number of times the associated \ref object
		 *	objects have had their move constructor invoked.
		 *
		 *	Note that in the case where a move constructor
		 *	throws it is still counted.
		 */
		std::size_t move_construct;
		/**
		 *	The number of times the associated \ref object
		 *	objects have had their copy assignment operator
		 *	invoked.
		 *
		 *	Note that in the case where a copy assignment
		 *	operator throws it is still counted.
		 */
		std::size_t copy_assign;
		/**
		 *	The number of times the associated \ref object
		 *	objects have had their move assignment operator
		 *	invoked.
		 *
		 *	Note that in the case where a move assignment
		 *	operator throws it is still counted.
		 */
		std::size_t move_assign;
		/**
		 *	The number of times the associated \ref object
		 *	objects have had their destructor invoked.
		 */
		std::size_t destruct;
		/**
		 *	If this member manages an exception that
		 *	exception shall be thrown whenever an
		 *	attempt is made to construct an assocaited
		 *	\ref object object through its constructor
		 *	which accepts an object of this type.
		 */
		std::exception_ptr construct_exception;
		/**
		 *	If this member manages an exception that
		 *	exception shall be thrown whenever an
		 *	attempt is made to copy construct an associated
		 *	\ref object object.
		 */
		std::exception_ptr copy_construct_exception;
		/**
		 *	If this member manages an exception that
		 *	exception shall be thrown whenever an attempt
		 *	is made to move construct an associated
		 *	\ref object object.
		 */
		std::exception_ptr move_construct_exception;
		/**
		 *	If this member manages an exception that
		 *	exception shall be thrown whenever an attempt
		 *	is made to copy assign an associated \ref object
		 *	object.
		 */
		std::exception_ptr copy_assign_exception;
		/**
		 *	If this member manages an exception that
		 *	exception shall be thrown whenever an attempt
		 *	is made to move assign an associated \ref object
		 *	object.
		 */
		std::exception_ptr move_assign_exception;
		state () noexcept;
		state (const state &) = default;
		state (state &&) = default;
		state & operator = (const state &) = default;
		state & operator = (state &&) = default; 
	};
private:
	state * state_;
public:
	object () = delete;
	object (const object &);
	object (object &&);
	object & operator = (const object &);
	object & operator = (object &&);
	/**
	 *	Creates a new object which counts calls
	 *	to its copy and more constructors, copy
	 *	and move assignment operators, and destructor,
	 *	and which decides whether those constructors
	 *	and assignment operators throw exceptions
	 *	based on a certain \ref state.
	 *
	 *	\param [in] s
	 *		A reference to the \ref state object
	 *		with which the newly-created object shall
	 *		be associated. This reference must remain
	 *		valid for the lifetime of the object or
	 *		the behavior is undefined.
	 */
	explicit object (state & s);
	~object () noexcept;
};

}
}
