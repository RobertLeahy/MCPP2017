/**
 *	\file
 */

#pragma once

#include "log.hpp"
#include <string>

namespace mcpp {

/**
 *	Derived from \ref log and does nothing.
 */
class null_log : public log {
protected:
	virtual void write_impl (const std::string &, std::string, level) override;
public:
	virtual bool ignored (level) override;
};

}
