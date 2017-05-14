/**
 *	\file
 */

#pragma once

#include <mcpp/optional.hpp>
#include <string>

namespace mcpp {
namespace yggdrasil {

/**
 *	Represents one of a user's profile.
 */
class profile {
public:
	profile ();
	profile (const profile &) = default;
	profile (profile &&) = default;
	profile & operator = (const profile &) = default;
	profile & operator = (profile &&) = default;
	/**
	 *	Creates a new profile.
	 *
	 *	\param [in] id
	 *		The profile's ID. Usually a UUID
	 *		without the dashes.
	 *	\param [in] name
	 *		The player name of that profile.
	 *	\param [in] legacy
	 *		\em true if the profile is a legacy
	 *		profile, \em false otherwise. Defaults
	 *		to \em false.
	 */
	profile (
		std::string id,
		std::string name,
		bool legacy = false
	);
	/**
	 *	The profile's ID. Usually a UUID without the
	 *	dashes.
	 */
	std::string id;
	/**
	 *	The player name of the profile.
	 */
	std::string name;
	/**
	 *	\em true of the profile is a legacy profile,
	 *	\em false otherwise.
	 */
	bool legacy;
};

}
}
