/**
 *	\file
 */

#pragma once

#include "error_code.hpp"
#include <boost/variant.hpp>
#include <cstddef>
#include <string>

namespace mcpp {
namespace protocol {

/**
 *	Indicates an error during protocol
 *	parsing.
 */
class error {
private:
	error_code c_;
	boost::variant<const char *, std::string> msg_;
	std::size_t offset_;
public:
	error () = delete;
	error (const error &) = default;
	error (error &&) = default;
	error & operator = (const error &) = default;
	error & operator = (error &&) = default;
	/**
	 *	Creates a new error object with an error
	 *	message chosen based on the provided
	 *	\ref error_code.
	 *
	 *	\param [in] c
	 *		The \ref error_code value.
	 *	\param [in] offset
	 *		The offset in bytes at which the error
	 *		occurred.
	 */
	error (error_code c, std::size_t offset);
	/**
	 *	Creates a new error object with a custom
	 *	error message.
	 *
	 *	The error message will not be copied and
	 *	it is therefore assumed that the lifetime
	 *	of the pointed to string shall be at least
	 *	as long as that of the newly created object
	 *	or any object created by copying or moving
	 *	it. If this assumption does not hold the
	 *	behavior in undefined.
	 *
	 *	\param [in] c
	 *		The \ref error_code value.
	 *	\param [in] str
	 *		A pointer to a null terminated string.
	 *	\param [in] offset
	 *		The offset in bytes at which the error
	 *		occurred.
	 */
	error (error_code c, const char * str, std::size_t offset) noexcept;
	/**
	 *	Creates a new error object with a custom
	 *	error message.
	 *
	 *	\param [in] c
	 *		The \ref error_code value.
	 *	\param [in] str
	 *		A string.
	 *	\param [in] offset
	 *		The offset in bytes at which the error
	 *		occurred.
	 */
	error (error_code c, std::string str, std::size_t offset);
	/**
	 *	Retrieves the message associated with this
	 *	error.
	 *
	 *	\return
	 *		A pointer to a null terminated string.
	 */
	const char * what () const noexcept;
	/**
	 *	Retrieves the error code associated with this
	 *	error.
	 *
	 *	\return
	 *		A \ref error_code value.
	 */
	error_code code () const noexcept;
	/**
	 *	Retrieves the offset in bytes at which the
	 *	error occurred.
	 *
	 *	\return
	 *		The offset in bytes.
	 */
	std::size_t offset () const noexcept;
	/**
	 *	Updates the offset.
	 *
	 *	\param [in] offset
	 *		The offset in bytes.
	 */
	void offset (std::size_t offset) noexcept;
};

}
}
