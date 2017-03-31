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
 *	Represents the abstract concept of a log.
 *
 *	A log is a write only data store which stores
 *	individual messages which have an associated
 *	"level" which indicates their importance,
 *	urgency, or type.
 */
class log {
public:
	/**
	 *	An enumeration of the levels which may
	 *	be associated with a message when it is
	 *	logged. 
	 */
	enum class level {
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
	static const std::string & to_string (level l);
protected:
	/**
	 *	Invoked by \ref write to write to the underlying
	 *	log.
	 *
	 *	\param [in] component
	 *		The name of the component which is writing to
	 *		the log.
	 *	\param [in] message
	 *		The message to write.
	 *	\param [in] l
	 *		The level associated with the message.
	 */
	virtual void write_impl (const std::string & component, std::string message, level l) = 0;
public:
	log () = default;
	log (const log &) = delete;
	log (log &&) = delete;
	log & operator = (const log &) = delete;
	log & operator = (log &&) = delete;
	/**
	 *	Allows derived classes to be cleaned up
	 *	through pointer or reference to base.
	 */
	virtual ~log () noexcept;
	/**
	 *	Writes a message with an associated component
	 *	to the log.
	 *
	 *	\param [in] component
	 *		The name of the component which is writing to
	 *		the log.
	 *	\param [in] message
	 *		The message to write.
	 *	\param [in] l
	 *		The level associated with the message. Defaults
	 *		to \ref level::info.
	 */
	void write (const std::string & component, std::string message, level l = level::info);
	/**
	 *	Determines whether the underlying logger is
	 *	ignoring a certain message level. In the case
	 *	where \em true is returned by this function
	 *	one may possibly improve efficiency by not
	 *	constructing a message to pass to \ref ref
	 *	since one knows it will simply be ignored.
	 *
	 *	\param [in] l
	 *		The log level to check.
	 *
	 *	\return
	 *		\em true if calling \ref write with \em l
	 *		will cause the associated message to simply
	 *		be ignored.
	 */
	virtual bool ignored (level l) = 0;
	/**
	 *	Invokes a functor which returns a std::string
	 *	and uses that result as a log message.
	 *
	 *	The functor shall only be invoked if invoking
	 *	\ref ignored with \em l returns \em false.
	 *
	 *	This method does not participate in overload
	 *	resolution unless \em F is a functor type which
	 *	may be invoked with no arguments to yield a
	 *	type convertible to std::string.
	 *
	 *	\tparam F
	 *		The type of functor to invoke.
	 *
	 *	\param [in] component
	 *		The name of the component which is writing to
	 *		the log.
	 *	\param [in] func
	 *		The functor to invoke.
	 *	\param [in] l
	 *		The level associated with the message. Defaults
	 *		to \ref level::info.
	 */
	template <typename F>
	#ifdef MCPP_DOXYGEN_RUNNING
	void
	#else
	std::enable_if_t<std::is_convertible<std::result_of_t<F ()>, std::string>::value>
	#endif
	write (const std::string & component, F && func, level l = level::info) {
		if (!ignored(l)) write_impl(component, func(), l);
	}
};

}

namespace std {

template <>
struct hash<mcpp::log::level> : private hash<underlying_type_t<mcpp::log::level>> {
private:
	using underlying_type = underlying_type_t<mcpp::log::level>;
	using base = hash<underlying_type>;
public:
	using result_type = std::size_t;
	using argument_type = mcpp::log::level;
	result_type operator () (mcpp::log::level l) const noexcept {
		return static_cast<const base &>(*this)(static_cast<underlying_type>(l));
	}
};

}
