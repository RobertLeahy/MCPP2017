/**
 *	\file
 */

#pragma once

#include "log.hpp"
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
	std::unordered_set<level> ignored_;
protected:
	virtual void write_impl (const std::string &, std::string, level) override;
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
	virtual bool ignored (level) override;
	/**
	 *	Ignores a level.
	 *
	 *	\param [in] l
	 *		The level to ignore.
	 */
	void ignore (level l);
};

}
