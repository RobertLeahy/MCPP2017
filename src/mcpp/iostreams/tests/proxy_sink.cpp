#include <mcpp/iostreams/proxy_sink.hpp>
#include <boost/core/ref.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/write.hpp>
#include <mcpp/buffer.hpp>
#include <algorithm>
#include <ios>
#include <iterator>
#include <catch.hpp>

namespace mcpp {
namespace iostreams {
namespace tests {
namespace {

class mock_sink {
private:
	bool closed_;
public:
	mock_sink () : closed_(false) {	}
	using char_type = char;
	class category
		:	public boost::iostreams::device_tag,
			public boost::iostreams::output,
			public boost::iostreams::closable_tag
	{	};
	std::streamsize write (const char_type *, std::streamsize) noexcept {
		return -1;
	}
	void close () noexcept {
		closed_ = true;
	}
	bool closed () const noexcept {
		return closed_;
	}
};

SCENARIO("mcpp::iostreams::proxy_sink passes through write operations to the wrapped Sink", "[mcpp][iostreams][proxy_sink]") {
	GIVEN("An mcpp::iostreams::proxy_sink which wraps a Sink") {
		char buf [5];
		buffer b(buf);
		auto ps = make_proxy_sink(boost::ref(b));
		WHEN("A write operation is performed thereupon") {
			char data [] = {'a', 'b', 'c', 'd', 'e', 'f'};
			auto result = boost::iostreams::write(ps, data, sizeof(data));
			THEN("The correct result is returned") {
				REQUIRE(result == 5);
				AND_THEN("The write is performed") {
					using std::begin;
					using std::end;
					CHECK(std::equal(begin(buf), end(buf), begin(data), begin(data) + 5));
				}
			}
		}
	}
}

SCENARIO("mcpp::iostreams::proxy_sink shields the wrapped Sink from being closed", "[mcpp][iostreams][proxy_sink]") {
	GIVEN("An mcpp::iostreams::proxy_sink which wraps a Sink") {
		mock_sink sink;
		auto ps = make_proxy_sink(boost::ref(sink));
		WHEN("It is closed") {
			boost::iostreams::close(ps, std::ios_base::out);
			THEN("The wrapped Sink is not closed") {
				CHECK_FALSE(sink.closed());
			}
		}
	}
}

}
}
}
}
