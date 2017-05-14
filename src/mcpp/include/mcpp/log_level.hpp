/**
 *	\file
 */

#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>

namespace mcpp {

/**
 *	An enumeration of the levels which may
 *	be associated with a message when it is
 *	logged. 
 */
enum class log_level {
	emergency,	/**<	System is unusable.	*/
	alert,	/**<	Action must be taken immediately.	*/
	critical,	/**<	Critical condition.	*/
	error,	/**<	Error condition.	*/
	warning,	/**<	Warning condition.	*/
	notice,	/**<	Normal but significant condition.	*/
	info,	/**<	Informational message.	*/
	debug	/**<	Message is for debugging purposes only.	*/
};
/**
 *	Obtains a human readable string which represents
 *	a log level.
 *
 *	\param [in] l
 *		The log level.
 *
 *	\return
 *		A string.
 */
const std::string & to_string (log_level l);

}

namespace std {

template <>
struct hash<mcpp::log_level> : private hash<underlying_type_t<mcpp::log_level>> {
private:
	using underlying_type = underlying_type_t<mcpp::log_level>;
	using base = hash<underlying_type>;
public:
	using result_type = std::size_t;
	using argument_type = mcpp::log_level;
	result_type operator () (argument_type l) const noexcept {
		return static_cast<const base &>(*this)(static_cast<underlying_type>(l));
	}
};

}
