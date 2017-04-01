/**
 *	\file
 */

#pragma once

#include "log.hpp"
#include "log_level.hpp"
#include <ostream>
#include <string>
#include <unordered_set>

namespace mcpp {

/**
 *	A concrete implementation of \ref log which
 *	writes messages to a std::ostream.
 */
class stream_log : public log {
private:
	std::ostream & os_;
	std::unordered_set<log_level> ignored_;
protected:
	virtual void write_impl (const std::string &, std::string, log_level) override;
public:
	stream_log () = delete;
	/**
	 *	Creates a new stream_log.
	 *
	 *	\param [in] os
	 *		The stream to which the newly
	 *		created object shall write.
	 */
	explicit stream_log (std::ostream & os) noexcept;
	virtual bool ignored (log_level) override;
	/**
	 *	Ignores a level.
	 *
	 *	\param [in] l
	 *		The level to ignore.
	 */
	void ignore (log_level l);
};

}
