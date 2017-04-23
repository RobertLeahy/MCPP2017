#include <mcpp/iostreams/concatenating_source.hpp>
#include <boost/core/ref.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/read.hpp>
#include <mcpp/buffer.hpp>
#include <ios>
#include <type_traits>
#include <catch.hpp>

namespace mcpp {
namespace iostreams {
namespace test {
namespace {

static_assert(
	std::is_same<concatenating_source<>::char_type, char>::value,
	"Incorrect char_type for concatenating_source with no underlying Sources"
);
static_assert(
	std::is_same<concatenating_source<buffer>::char_type, char>::value,
	"Incorrect char_type for concatenating_source with one underlying Source"
);
static_assert(
	std::is_same<concatenating_source<buffer, buffer>::char_type, char>::value,
	"Incorrect char_type for concatenating_source with two underlying Sources"
);

class mock_source {
private:
	bool closed_;
public:
	mock_source () noexcept : closed_(false) {	}
	using char_type = char;
	class category
		:	public boost::iostreams::device_tag,
			public boost::iostreams::input,
			public boost::iostreams::closable_tag
	{	};
	std::streamsize read (const char_type *, std::streamsize) noexcept {
		return -1;
	}
	void close () noexcept {
		closed_ = true;
	}
	bool closed () const noexcept {
		return closed_;
	}
};

SCENARIO("mcpp::iostreams::concatenating_source forms a character sequence which is the concatenation of the character sequences managed by its child Sources", "[mcpp][iostreams][concatenating_source]") {
	GIVEN("An mcpp::iostreams::concatenating_source with no underlying Source") {
		auto src = make_concatenating_source();
		WHEN("An attempt is made to read therefrom") {
			decltype(src)::char_type buffer [4];
			auto res = boost::iostreams::read(src, buffer, sizeof(buffer));
			THEN("End of stream is encountered") {
				CHECK(res == -1);
			}
		}
	}
	GIVEN("An mcpp::iostreams::concatenating_source with one underlying Source") {
		unsigned char buf [] = {1, 2, 3, 4};
		buffer b(buf);
		auto src = make_concatenating_source(b);
		WHEN("An attempt is made to read therefrom") {
			char rbuf [3];
			auto res = boost::iostreams::read(src, rbuf, 2);
			THEN("The requested number of characters are read") {
				REQUIRE(res == 2);
				AND_THEN("The correct characters are read") {
					CHECK(rbuf[0] == 1);
					CHECK(rbuf[1] == 2);
				}
			}
			AND_WHEN("The remainder of the stream is read") {
				res = boost::iostreams::read(src, rbuf, sizeof(rbuf));
				THEN("A number of characters equal to the number remaining in the stream are read") {
					REQUIRE(res == 2);
					AND_THEN("The correct characters are read") {
						CHECK(rbuf[0] == 3);
						CHECK(rbuf[1] == 4);
					}
				}
				AND_WHEN("An attempt is made to read the stream again") {
					res = boost::iostreams::read(src, rbuf, 1);
					THEN("End of stream is encountered") {
						CHECK(res == -1);
					}
				}
			}
		}
	}
	GIVEN("An mcpp::iostreams::concatenating_source with two underlying Sources") {
		unsigned char buf1 [] = {1, 2, 3};
		buffer b1(buf1);
		unsigned char buf2 [] = {4, 5};
		buffer b2(buf2);
		auto src = make_concatenating_source(b1, b2);
		WHEN("An attempt is made to read therefrom which reads only from the first Source") {
			char rbuf [2];
			auto res = boost::iostreams::read(src, rbuf, sizeof(rbuf));
			THEN("The requested number of characters are read") {
				REQUIRE(res == 2);
				AND_THEN("The correct characters are read") {
					CHECK(rbuf[0] == 1);
					CHECK(rbuf[1] == 2);
				}
			}
			AND_WHEN("An attempt is made to read therefrom which reads both from the first and second Source") {
				res = boost::iostreams::read(src, rbuf, sizeof(rbuf));
				THEN("The requested number of characters are read") {
					REQUIRE(res == 2);
					AND_THEN("The correct characters are read") {
						CHECK(rbuf[0] == 3);
						CHECK(rbuf[1] == 4);
					}
				}
				AND_WHEN("The remainder of the characters managed by the second Source are read") {
					res = boost::iostreams::read(src, rbuf, sizeof(rbuf));
					THEN("The requested number of characters are read") {
						REQUIRE(res == 1);
						AND_THEN("The correct characters are read") {
							CHECK(rbuf[0] == 5);
						}
					}
					AND_WHEN("An attempt is made to read the stream again") {
						res = boost::iostreams::read(src, rbuf, 1);
						THEN("End of stream is encountered") {
							CHECK(res == -1);
						}
					}
				}
			}
		}
	}
}

SCENARIO("mcpp::iostreams::concatenating_source may be closed", "[mcpp][iostreams][concatenating_source]") {
	GIVEN("An mcpp::iostreams::concatenating_source with multiple child Sources") {
		mock_source a;
		mock_source b;
		auto src = make_concatenating_source(boost::ref(a), boost::ref(b));
		WHEN("It is closed") {
			boost::iostreams::close(src, std::ios_base::in);
			THEN("The underlying Sources are closed") {
				CHECK(a.closed());
				CHECK(b.closed());
			}
		}
	}
}

}
}
}
}
