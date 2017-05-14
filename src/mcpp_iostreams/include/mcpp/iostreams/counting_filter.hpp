/**
 *	\file
 */

#pragma once

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/write.hpp>
#include <cstddef>
#include <ios>

namespace mcpp {
namespace iostreams {

/**
 *	Models `BidirectionalFilter` and counts
 *	the number of characters which pass through it.
 *
 *	Note that separate counts are not maintained for
 *	input and output. The total number of characters
 *	passing through the filter in either direction
 *	shall be aggregated in a single count.
 *
 *	\tparam CharT
 *		The character type of the filter.
 */
template <typename CharT>
class basic_counting_filter {
private:
	std::size_t count_;
public:
	basic_counting_filter () noexcept : count_(0) {	}
	basic_counting_filter (const basic_counting_filter &) = default;
	basic_counting_filter (basic_counting_filter &&) = default;
	basic_counting_filter & operator = (const basic_counting_filter &) = default;
	basic_counting_filter & operator = (basic_counting_filter &&) = default;
	using char_type = CharT;
	using category = boost::iostreams::multichar_input_filter_tag;
	template <typename Device>
	std::streamsize read (Device & d, CharT * s, std::streamsize n) {
		auto retr = boost::iostreams::read(d, s, n);
		if (retr > 0) count_ += std::size_t(retr);
		return retr;
	}
	template <typename Device>
	std::streamsize write (Device & d, const CharT * s, std::streamsize n) {
		auto retr = boost::iostreams::write(d, s, n);
		if (retr > 0) count_ += std::size_t(retr);
		return retr;
	}
	/**
	 *	Obtains the number of characters which have
	 *	passed through the filter.
	 *
	 *	\return
	 *		The count.
	 */
	std::size_t count () const noexcept {
		return count_;
	}
	/**
	 *	Resets the count to zero.
	 */
	void reset () noexcept {
		count_ = 0;
	}
};

/**
 *	A type alias for \ref basic_counting_filter
 *	templated on `char`.
 */
using counting_filter = basic_counting_filter<char>;

}
}
