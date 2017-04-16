/**
 *	\file
 */

#pragma once

#include "packet.hpp"
#include <cstddef>
#include <stdexcept>
#include <typeinfo>

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

/**
 *	Indicates that a \ref packet_serializer could not be
 *	found matching the runtime type of a \ref packet.
 */
class packet_serializer_not_found : public serialize_error {
private:
	const std::type_info * type_;
public:
	/**
	 *	Creates a new packet_serializer_not_found
	 *	by capturing the `std::type_info` associated
	 *	with the runtime type of a \ref packet.
	 *
	 *	\param [in] p
	 *		A reference to the \ref packet whose
	 *		runtime type shall be queried and stored.
	 *		Note that this reference need not remain
	 *		valid after this constructor completes.
	 */
	explicit packet_serializer_not_found (const packet & p);
	/**
	 *	Creates a new packet_serializer_not_found
	 *	directly from a `std::type_info` representing
	 *	the type of the \ref packet a \ref packet_serializer
	 *	for which could not be found.
	 *
	 *	\param [in] type
	 *		A reference to a `std::type_info` structure.
	 */
	explicit packet_serializer_not_found (const std::type_info & type);
	/**
	 *	Retrieves the `std::type_info` structure which
	 *	represents the runtime type of the \ref packet
	 *	for which a \ref packet_serializer could not be
	 *	found.
	 *
	 *	\return
	 *		A `std::type_info`.
	 */
	const std::type_info & type () const noexcept;
};

}
}
