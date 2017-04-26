#include <mcpp/protocol/stream_serializer.hpp>
#include <boost/core/ref.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <mcpp/buffer.hpp>
#include <mcpp/iostreams/concatenating_source.hpp>
#include <mcpp/iostreams/proxy_sink.hpp>
#include <mcpp/protocol/direction.hpp>
#include <mcpp/protocol/error.hpp>
#include <mcpp/protocol/exception.hpp>
#include <mcpp/protocol/handshaking.hpp>
#include <mcpp/protocol/packet.hpp>
#include <mcpp/protocol/packet_serializer_map.hpp>
#include <mcpp/protocol/state.hpp>
#include <mcpp/protocol/varint.hpp>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <catch.hpp>

namespace mcpp {
namespace protocol {
namespace tests {
namespace {

SCENARIO("Uncompressed packets may be parsed", "[mcpp][protocol][stream_serializer]") {
	GIVEN("An mcpp::protocol::stream_serializer") {
		using stream_serializer_type = stream_serializer<buffer, buffer>;
		stream_serializer_type ser(
			packet_serializer_map<
				stream_serializer_type::inner_source_type,
				stream_serializer_type::inner_sink_type
			>(),
			direction::serverbound
		);
		WHEN("A packet is parsed") {
			unsigned char buf [] = {
				11,
				0,
				0b10111100, 0b00000010,
				4, 't', 'e', 's', 't',
				0b01100011, 0b11011101,
				1
			};
			buffer b(buf);
			auto result = ser.parse(b);
			THEN("The parse completes successfully") {
				REQUIRE(result);
				AND_THEN("mcpp::protocol::stream_serializer::cached reports the correct value") {
					CHECK(ser.cached() == sizeof(buf));
				}
				AND_THEN("The last packet parsed was not compressed") {
					CHECK_FALSE(ser.parsed_compressed());
				}
				AND_THEN("The size of the last parsed packet's body is correctly reported") {
					CHECK(ser.parsed_size() == 11);
					CHECK_FALSE(ser.parsed_empty());
				}
				AND_THEN("A packet is parsed") {
					REQUIRE(*result);
					REQUIRE(ser.has_packet());
					AND_THEN("The parsed packet is correct") {
						auto && p = dynamic_cast<const handshaking::serverbound::handshake &>(ser.packet());
						CHECK(p.protocol_version == 316);
						CHECK(p.server_address == "test");
						CHECK(p.server_port == 25565);
						CHECK(p.next_state == state::status);
					}
					AND_THEN("The parsed packet ID is correct") {
						auto id = ser.id();
						CHECK(id.id() == 0);
						CHECK(id.direction() == direction::serverbound);
						CHECK(id.state() == state::handshaking);
					}
					AND_THEN("The parsed body of the packet may be accessed") {
						unsigned char expected [] = {
							0,
							0b10111100, 0b00000010,
							4, 't', 'e', 's', 't',
							0b01100011, 0b11011101,
							1
						};
						unsigned char buf [128];
						buffer b(buf);
						auto src = ser.parsed();
						auto num = boost::iostreams::copy(src, b);
						using std::begin;
						using std::end;
						CHECK(std::equal(begin(buf), begin(buf) + num, begin(expected), end(expected)));
					}
				}
			}
		}
		WHEN("A packet which claims to be shorter than its representation is parsed") {
			unsigned char buf [] = {
				10,	//	Should be 11
				0,
				0b10111100, 0b00000010,
				4, 't', 'e', 's', 't',
				0b01100011, 0b11011101,
				1
			};
			buffer b(buf);
			auto result = ser.parse(b);
			THEN("The parse does not complete successfully") {
				REQUIRE_FALSE(result);
				CHECK(result.error() == make_error_code(error::end_of_file));
			}
			THEN("The portion of the packet representation not included in the length prefix is not read") {
				CHECK(b.read() == (sizeof(buf) - 1));
			}
		}
		WHEN("A packet which claims to be longer than its representation is parsed") {
			unsigned char buf [] = {
				12,	//	Should be 11
				0,
				0b10111100, 0b00000010,
				4, 't', 'e', 's', 't',
				0b01100011, 0b11011101,
				1,
				0	//	Extra character
			};
			buffer b(buf);
			auto result = ser.parse(b);
			THEN("The parse does not complete successfully") {
				REQUIRE_FALSE(result);
				CHECK(result.error() == make_error_code(error::inconsistent_length));
			}
		}
		WHEN("An unknown packet is parsed") {
			unsigned char buf [] = {
				10,
				127,	//	No packet 127 in handshaking
				0, 0, 0, 0, 0, 0, 0, 0, 0
			};
			buffer b(buf);
			auto result = ser.parse(b);
			THEN("The parse completes successfully") {
				REQUIRE(result);
				AND_THEN("mcpp::protocol::stream_serializer::cached reports the correct value") {
					CHECK(ser.cached() == sizeof(buf));
				}
				AND_THEN("The last packet parsed was not compressed") {
					CHECK_FALSE(ser.parsed_compressed());
				}
				AND_THEN("The size of the last parsed packet's body is correctly reported") {
					CHECK(ser.parsed_size() == 10);
					CHECK_FALSE(ser.parsed_empty());
				}
				AND_THEN("A packet is parsed") {
					REQUIRE(*result);
					AND_THEN("The mcpp::protocol::stream_serializer does not manage a packet") {
						CHECK(!ser.has_packet());
					}
					AND_THEN("The parsed packet ID is correct") {
						auto && id = ser.id();
						CHECK(id.id() == 127);
						CHECK(id.direction() == direction::serverbound);
						CHECK(id.state() == state::handshaking);
					}
					AND_THEN("The parsed body of the packet may be accessed") {
						unsigned char expected [] = {
							127,
							0, 0, 0, 0, 0, 0, 0, 0, 0
						};
						unsigned char buf [128];
						buffer b(buf);
						auto src = ser.parsed();
						auto num = boost::iostreams::copy(src, b);
						using std::begin;
						using std::end;
						CHECK(std::equal(begin(buf), begin(buf) + num, begin(expected), end(expected)));
					}
				}
			}
		}
	}
}

SCENARIO("Compressed packets may be parsed", "[mcpp][protocol][stream_serializer]") {
	GIVEN("An mcpp::protocol::stream_serializer") {
		using vectorbuf_type = boost::interprocess::basic_vectorbuf<
			std::vector<char>,
			std::char_traits<char>
		>;
		using stream_serializer_type = stream_serializer<
			iostreams::concatenating_source<
				boost::reference_wrapper<vectorbuf_type>,
				boost::reference_wrapper<vectorbuf_type>,
				boost::reference_wrapper<vectorbuf_type>
			>,
			buffer
		>;
		stream_serializer_type ser(
			packet_serializer_map<
				stream_serializer_type::inner_source_type,
				stream_serializer_type::inner_sink_type
			>(),
			direction::serverbound
		);
		ser.enable_compression(0);	//	Compress everything
		WHEN("A compressed packet is parsed") {
			unsigned char body [] = {
				0,
				0b10111100, 0b00000010,
				4, 't', 'e', 's', 't',
				0b01100011, 0b11011101,
				1
			};
			buffer b(body);
			vectorbuf_type body_compressed;
			boost::iostreams::copy(
				boost::iostreams::compose(
					boost::iostreams::zlib_compressor{},
					//	Otherwise boost complains about dual use filters
					iostreams::make_concatenating_source(boost::ref(b))
				),
				body_compressed
			);
			vectorbuf_type body_length;
			serialize_varint(sizeof(body), body_length);
			vectorbuf_type length;
			serialize_varint(
				body_length.vector().size() + body_compressed.vector().size(),
				length
			);
			auto source = iostreams::make_concatenating_source(
				boost::ref(length),
				boost::ref(body_length),
				boost::ref(body_compressed)
			);
			auto result = ser.parse(source);
			THEN("The parse completes successfully") {
				REQUIRE(result);
				AND_THEN("mcpp::protocol::stream_serializer::cached reports the correct number of characters") {
					CHECK(ser.cached() == (length.vector().size() + body_length.vector().size() + body_compressed.vector().size()));
				}
				AND_THEN("The last packet parsed was compressed") {
					CHECK(ser.parsed_compressed());
					AND_THEN("The size of the last parsed packet's compressed body is correctly reported") {
						CHECK(ser.parsed_compressed_size() == body_compressed.vector().size());
					}
				}
				AND_THEN("The size of the last parsed packet's body is correctly reported") {
					CHECK(ser.parsed_size() == 11);
					CHECK_FALSE(ser.parsed_empty());
				}
				AND_THEN("A packet is parsed") {
					REQUIRE(*result);
					REQUIRE(ser.has_packet());
					AND_THEN("The parsed packet is correct") {
						auto && p = dynamic_cast<const handshaking::serverbound::handshake &>(ser.packet());
						CHECK(p.protocol_version == 316);
						CHECK(p.server_address == "test");
						CHECK(p.server_port == 25565);
						CHECK(p.next_state == state::status);
					}
					AND_THEN("The parsed packet ID is correct") {
						auto id = ser.id();
						CHECK(id.id() == 0);
						CHECK(id.direction() == direction::serverbound);
						CHECK(id.state() == state::handshaking);
					}
					AND_THEN("The parsed body of the packet may be accessed") {
						unsigned char expected [] = {
							0,
							0b10111100, 0b00000010,
							4, 't', 'e', 's', 't',
							0b01100011, 0b11011101,
							1
						};
						unsigned char buf [128];
						buffer b(buf);
						auto src = ser.parsed();
						auto num = boost::iostreams::copy(src, b);
						using std::begin;
						using std::end;
						CHECK(std::equal(begin(buf), begin(buf) + num, begin(expected), end(expected)));
					}
				}
			}
		}
		WHEN("An uncompressed packet is parsed") {
			vectorbuf_type a;
			vectorbuf_type b;
			vectorbuf_type c;
			unsigned char buf [] = {
				12,
				0,
				0,
				0b10111100, 0b00000010,
				4, 't', 'e', 's', 't',
				0b01100011, 0b11011101,
				1
			};
			using std::begin;
			using std::end;
			std::vector<char> vec(begin(buf), end(buf));
			c.swap_vector(vec);
			auto source = iostreams::make_concatenating_source(
				boost::ref(a),
				boost::ref(b),
				boost::ref(c)
			);
			ser.enable_compression(1000);
			auto result = ser.parse(source);
			THEN("The parse completes successfully") {
				REQUIRE(result);
				AND_THEN("mcpp::protocol::stream_serializer::cached returns the correct value") {
					CHECK(ser.cached() == sizeof(buf));
				}
				AND_THEN("The last packet parsed was not compressed") {
					CHECK_FALSE(ser.parsed_compressed());
				}
				AND_THEN("The size of the last parsed packet's body is correctly reported") {
					CHECK(ser.parsed_size() == 11);
					CHECK_FALSE(ser.parsed_empty());
				}
				AND_THEN("A packet is parsed") {
					REQUIRE(*result);
					REQUIRE(ser.has_packet());
					AND_THEN("The parsed packet is correct") {
						auto && p = dynamic_cast<const handshaking::serverbound::handshake &>(ser.packet());
						CHECK(p.protocol_version == 316);
						CHECK(p.server_address == "test");
						CHECK(p.server_port == 25565);
						CHECK(p.next_state == state::status);
					}
					AND_THEN("The parsed packet ID is correct") {
						auto id = ser.id();
						CHECK(id.id() == 0);
						CHECK(id.direction() == direction::serverbound);
						CHECK(id.state() == state::handshaking);
					}
					AND_THEN("The parsed body of the packet may be accessed") {
						unsigned char expected [] = {
							0,
							0b10111100, 0b00000010,
							4, 't', 'e', 's', 't',
							0b01100011, 0b11011101,
							1
						};
						unsigned char buf [128];
						buffer b(buf);
						auto src = ser.parsed();
						auto num = boost::iostreams::copy(src, b);
						using std::begin;
						using std::end;
						CHECK(std::equal(begin(buf), begin(buf) + num, begin(expected), end(expected)));
					}
				}
			}
		}
		WHEN("A compressed packet is parsed but its body is too short to qualify for compression") {
			unsigned char body [] = {
				0,
				0b10111100, 0b00000010,
				4, 't', 'e', 's', 't',
				0b01100011, 0b11011101,
				1
			};
			buffer b(body);
			vectorbuf_type body_compressed;
			boost::iostreams::copy(
				boost::iostreams::compose(
					boost::iostreams::zlib_compressor{},
					//	Otherwise boost complains about dual use filters
					iostreams::make_concatenating_source(boost::ref(b))
				),
				body_compressed
			);
			vectorbuf_type body_length;
			serialize_varint(sizeof(body), body_length);
			vectorbuf_type length;
			serialize_varint(
				body_length.vector().size() + body_compressed.vector().size(),
				length
			);
			auto source = iostreams::make_concatenating_source(
				boost::ref(length),
				boost::ref(body_length),
				boost::ref(body_compressed)
			);
			ser.enable_compression(1000);
			auto result = ser.parse(source);
			THEN("The parse fails") {
				REQUIRE_FALSE(result);
				CHECK(result.error() == make_error_code(error::compressed));
			}
		}
		WHEN("An uncompressed packet is parsed but its body is too long to be uncompressed") {
			vectorbuf_type a;
			vectorbuf_type b;
			vectorbuf_type c;
			unsigned char buf [] = {
				12,
				0,
				0,
				0b10111100, 0b00000010,
				4, 't', 'e', 's', 't',
				0b01100011, 0b11011101,
				1
			};
			using std::begin;
			using std::end;
			std::vector<char> vec(begin(buf), end(buf));
			c.swap_vector(vec);
			auto source = iostreams::make_concatenating_source(
				boost::ref(a),
				boost::ref(b),
				boost::ref(c)
			);
			auto result = ser.parse(source);
			THEN("The parse fails") {
				REQUIRE_FALSE(result);
				CHECK(result.error() == make_error_code(error::uncompressed));
			}
		}
	}
}

SCENARIO("mcpp::protocol::packet objects may be serialized to an uncompressed representation", "[mcpp][protocol][stream_serializer]") {
	GIVEN("An mcpp::protocol::stream_serializer") {
		using stream_serializer_type = stream_serializer<buffer, buffer>;
		stream_serializer_type ser(
			packet_serializer_map<
				stream_serializer_type::inner_source_type,
				stream_serializer_type::inner_sink_type
			>(),
			direction::serverbound
		);
		WHEN("An mcpp::protocol::packet is serialized") {
			handshaking::serverbound::handshake p;
			p.protocol_version = 316;
			p.server_address = "test";
			p.server_port = 25565;
			p.next_state = state::status;
			unsigned char buf [128];
			buffer b(buf);
			ser.serialize(p, b);
			THEN("The correct representation is written to the provided Sink") {
				unsigned char expected [] = {
					11,
					0,
					0b10111100, 0b00000010,
					4, 't', 'e', 's', 't',
					0b01100011, 0b11011101,
					1
				};
				using std::begin;
				using std::end;
				CHECK(std::equal(begin(expected), end(expected), begin(buf), begin(buf) + b.written()));
			}
		}
		WHEN("An mcpp::protocol::packet for which there is no mcpp::protocol::packet_serializer is serialized") {
			class : public packet {	} p;
			unsigned char buf [128];
			buffer b(buf);
			THEN("An mcpp::protocol::packet_serializer_not_found exception is thrown") {
				CHECK_THROWS_AS(ser.serialize(p, b), packet_serializer_not_found);
			}
		}
	}
}

SCENARIO("mcpp::protocol::packet objects may be serialized to a compressed representation", "[mcpp][protocol][stream_serializer]") {
	GIVEN("An mcpp::protocol::stream_serializer") {
		using stream_serializer_type = stream_serializer<buffer, buffer>;
		stream_serializer_type ser(
			packet_serializer_map<
				stream_serializer_type::inner_source_type,
				stream_serializer_type::inner_sink_type
			>(),
			direction::serverbound
		);
		WHEN("A packet whose length is below the threshold is serialized") {
			ser.enable_compression(12);
			handshaking::serverbound::handshake p;
			p.protocol_version = 316;
			p.server_address = "test";
			p.server_port = 25565;
			p.next_state = state::status;
			unsigned char buf [128];
			buffer b(buf);
			ser.serialize(p, b);
			THEN("It is not compressed") {
				CHECK_FALSE(ser.serialized_compressed());
			}
			THEN("The serialized body is correct") {
				CHECK_FALSE(ser.serialized_empty());
				unsigned char expected [] = {
					0,
					0b10111100, 0b00000010,
					4, 't', 'e', 's', 't',
					0b01100011, 0b11011101,
					1
				};
				CHECK(ser.serialized_size() == sizeof(expected));
				unsigned char buf [128];
				buffer b(buf);
				auto source = ser.serialized();
				boost::iostreams::copy(
					source,
					b
				);
				using std::begin;
				using std::end;
				CHECK(std::equal(begin(expected), end(expected), begin(buf), begin(buf) + b.written()));
			}
			THEN("The correct representation is written") {
				unsigned char expected [] = {
					12,
					0,
					0,
					0b10111100, 0b00000010,
					4, 't', 'e', 's', 't',
					0b01100011, 0b11011101,
					1
				};
				using std::begin;
				using std::end;
				CHECK(std::equal(begin(expected), end(expected), begin(buf), begin(buf) + b.written()));
			}
		}
		WHEN("A packet whose length is equal to or above the threshold is serialized") {
			ser.enable_compression(11);
			handshaking::serverbound::handshake p;
			p.protocol_version = 316;
			p.server_address = "test";
			p.server_port = 25565;
			p.next_state = state::status;
			unsigned char buf [128];
			buffer b(buf);
			ser.serialize(p, b);
			THEN("The serialized body is correct") {
				CHECK_FALSE(ser.serialized_empty());
				unsigned char expected [] = {
					0,
					0b10111100, 0b00000010,
					4, 't', 'e', 's', 't',
					0b01100011, 0b11011101,
					1
				};
				CHECK(ser.serialized_size() == sizeof(expected));
				unsigned char buf [128];
				buffer b(buf);
				auto source = ser.serialized();
				boost::iostreams::copy(
					source,
					b
				);
				using std::begin;
				using std::end;
				CHECK(std::equal(begin(expected), end(expected), begin(buf), begin(buf) + b.written()));
			}
			THEN("The correct representation is written") {
				unsigned char body_buffer [] = {
					0,
					0b10111100, 0b00000010,
					4, 't', 'e', 's', 't',
					0b01100011, 0b11011101,
					1
				};
				buffer body(body_buffer);
				unsigned char compressed_body_buffer [128];
				buffer compressed_body(compressed_body_buffer);
				boost::iostreams::copy(
					body,
					boost::iostreams::compose(
						boost::iostreams::zlib_compressor{},
						iostreams::make_proxy_sink(boost::ref(compressed_body))
					)
				);
				unsigned char length_b_buffer = 11;
				unsigned char length_a_buffer [5];
				buffer length_a(length_a_buffer);
				serialize_varint(compressed_body.written() + 1, length_a);
				unsigned char expected_buffer [128];
				buffer expected(expected_buffer);
				boost::iostreams::copy(
					iostreams::make_concatenating_source(
						buffer(length_a_buffer, length_a.written()),
						buffer(&length_b_buffer, 1),
						buffer(compressed_body_buffer, compressed_body.written())
					),
					expected
				);
				using std::begin;
				using std::end;
				CHECK(std::equal(begin(expected_buffer), begin(expected_buffer) + expected.written(), begin(buf), begin(buf) + b.written()));
				//	We structure the test with this in here
				//	rather than at the level above because we
				//	need to know the length of the compressed
				//	body
				AND_THEN("It is compressed") {
					REQUIRE(ser.serialized_compressed());
					AND_THEN("The compressed length is reported correctly") {
						CHECK(ser.serialized_compressed_size() == compressed_body.written());
					}
				}
			}
		}
	}
}

}
}
}
}
