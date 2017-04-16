#include <mcpp/iostreams/proxy_source.hpp>
#include <boost/core/ref.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/read.hpp>
#include <mcpp/buffer.hpp>
#include <ios>
#include <catch.hpp>

namespace mcpp {
namespace iostreams {
namespace tests {
namespace {

class mock_source {
private:
	bool closed_;
public:
	mock_source () : closed_(false) {	}
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

SCENARIO("mcpp::iostreams::proxy_source passes through read operations to the wrapped Source", "[mcpp][iostreams][proxy_source]") {
	GIVEN("An mcpp::iostreams::proxy_source which wraps a Source") {
		const unsigned char buf [] = {1, 2, 3, 4};
		buffer b(buf);
		auto ps = make_proxy_source(boost::ref(b));
		WHEN("A read operation is performed thereupon") {
			char data [2];
			auto result = boost::iostreams::read(ps, data, sizeof(data));
			THEN("The correct result is returned") {
				REQUIRE(result == 2);
				AND_THEN("The correct data is read") {
					CHECK(data[0] == 1);
					CHECK(data[1] == 2);
				}
			}
		} 
	}
}

SCENARIO("mcpp::iostreams::proxy_source shields the wrapped Source from being closed", "[mcpp][iostreams][proxy_source]") {
	GIVEN("An mcpp::iostreams::proxy_source which wraps a Source") {
		mock_source src;
		auto ps = make_proxy_source(boost::ref(src));
		WHEN("It is closed") {
			boost::iostreams::close(ps, std::ios_base::in);
			THEN("The wrapped Sink is not closed") {
				CHECK_FALSE(src.closed());
			}
		}
	}
}

}
}
}
}
