/**
 *	\file
 */

#pragma once

#include <cstddef>
#include <stdexcept>

namespace mcpp {
namespace protocol {

/**
 *	A base class for protocol-related exceptions.
 */
class exception : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
};

/**
 *	Indicates an error occurred when attempting
 *	to serialize data.
 */
class serialize_error : public exception {
public:
	using exception::exception;
};

/**
 *	Indicates an error occurred when attempting
 *	to write data to a stream.
 */
class write_error : public serialize_error {
public:
	using serialize_error::serialize_error;
};

/**
 *	Indicates that an attempt was made to write
 *	a certain number of bytes to a stream but
 *	the stream only allowed a lesser number of
 *	bytes to be written.
 */
class write_overflow_error : public write_error {
public:
	/**
	 *	Creates a new write_overflow_error.
	 *
	 *	\param [in] attempted
	 *		The number of bytes which were to be
	 *		written.
	 *	\param [in] actual
	 *		The number of bytes which were actually
	 *		written.
	 */
	write_overflow_error (std::size_t attempted, std::size_t actual);
};

/**
 *	Indicates that a certain value was not representable.
 */
class unrepresentable_error : public serialize_error {
public:
	using serialize_error::serialize_error;
};

}
}
