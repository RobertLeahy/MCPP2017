/**
 *	\file
 */

#pragma once

#include "log.hpp"
#include "log_level.hpp"
#include <string>

namespace mcpp {

/**
 *	Derived from \ref log and does nothing.
 */
class null_log : public log {
protected:
	virtual void write_impl (const std::string &, std::string, log_level) override;
public:
	virtual bool ignored (log_level) override;
};

}
