/**
 *	\file
 */

#pragma once

#include "direction.hpp"
#include "error.hpp"
#include "exception.hpp"
#include "incremental_varint_parser.hpp"
#include "packet.hpp"
#include "packet_id.hpp"
#include "packet_serializer.hpp"
#include "packet_serializer_map_t.hpp"
#include "state.hpp"
#include "varint.hpp"
#include <boost/core/ref.hpp>
#include <boost/expected/expected.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <mcpp/buffer.hpp>
#include <mcpp/checked.hpp>
#include <mcpp/iostreams/concatenating_source.hpp>
#include <mcpp/iostreams/limiting_source.hpp>
#include <mcpp/iostreams/offset.hpp>
#include <mcpp/iostreams/proxy_sink.hpp>
#include <mcpp/iostreams/proxy_source.hpp>
#include <mcpp/iostreams/traits.hpp>
#include <mcpp/optional.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>
#include <sstream>
#include <system_error>
#include <utility>
#include <vector>

namespace mcpp {
namespace protocol {

template <typename Source, typename Sink, typename Allocator = std::allocator<packet>>
class stream_serializer {
//
//	Shared
//
public:
	/**
	 *	The type supplied as the \em Source template
	 *	parameter.
	 */
	using source_type = Source;
	/**
	 *	The type supplied as the \em Sink template
	 *	parameter.
	 */
	using sink_type = Sink;
	/**
	 *	The type supplied as the \em Allocator
	 *	template parameter.
	 */
	using allocator_type = Allocator;
	/**
	 *	The `Allocator` type which should be used
	 *	when acquiring a \ref packet_serializer_map_t.
	 */
	using inner_allocator_type = Allocator;
private:
	template <typename Device>
	using vectorbuf_allocator_t = typename std::allocator_traits<
		inner_allocator_type
	>::template rebind_alloc<
		iostreams::char_type_of_t<Device>
	>;
	template <typename Device>
	using vectorbuf_t = boost::interprocess::basic_vectorbuf<
		std::vector<iostreams::char_type_of_t<Device>, vectorbuf_allocator_t<Device>>,
		iostreams::traits_of_t<Device>
	>;
	using size_type = std::uint32_t;
public:
	/**
	 *	The `Source` which should be used when
	 *	acquiring a \ref packet_serializer_map_t.
	 */
	using inner_source_type = vectorbuf_t<Source>;
	/**
	 *	The `Sink` which should be used when
	 *	acquiring a \ref packet_serializer_map_t.
	 */
	using inner_sink_type = vectorbuf_t<Sink>;
	/**
	 *	The exact instantiation of \ref packet_serializer_map_t
	 *	which objects of this type shall use to look up
	 *	\ref packet_serializer objects.
	 */
	using packet_serializer_map_type = packet_serializer_map_t<
		inner_source_type,
		inner_sink_type,
		inner_allocator_type
	>;
private:
	using inner_source_vector_type = typename inner_source_type::vector_type;
	using inner_source_allocator_type = vectorbuf_allocator_t<Source>;
	using inner_sink_vector_type = typename inner_sink_type::vector_type;
	using inner_sink_allocator_type = vectorbuf_allocator_t<Sink>;
	packet_serializer_map_type map_;
	protocol::direction direction_;
	protocol::state state_;
	optional<std::size_t> threshold_;
//
//	Parse
//
private:
	using source_char_type = iostreams::char_type_of_t<Source>;
	using size_parser_type = incremental_varint_parser<size_type, source_char_type>;
	using pointer = typename packet_serializer<
		inner_source_type,
		inner_sink_type,
		inner_allocator_type
	>::pointer;
	//	TODO: Make this able to accept an allocator,
	//	currently it seems that basic_zlib_decompressor
	//	is incapable of accepting a stateful allocator
	using decompressor_type = boost::iostreams::zlib_decompressor;
public:
	/**
	 *	The type returned by the \ref parse method.
	 *
	 *	May hold a boolean in which case it represents
	 *	whether the parse operation succeeded (\em true)
	 *	or not (\em false).
	 *
	 *	May also hold a `std::error_code` in which case
	 *	the parse operation failed due to an error.
	 */
	using parse_result_type = boost::expected<bool, std::error_code>;
	/**
	 *	A type which models `Source` an instance of which
	 *	is returned by this \ref parsed method.
	 */
	using parsed_source = buffer;
private:
	inner_source_type parse_body_;
	size_parser_type parse_size_a_;
	size_parser_type parse_size_b_;
	optional<packet_id> parse_packet_id_;
	pointer parse_pointer_;
	decompressor_type parse_decompressor_;
	std::size_t parse_body_consumed_;
	std::size_t parse_body_compressed_size_;
	template <typename Source2>
	parse_result_type parse_body (Source2 & src, std::uint32_t n) {
		//	TODO: Safe conversion to std::size_t?
		std::size_t size(n);
		auto limiting = iostreams::make_limiting_source(
			boost::ref(src),
			size - parse_body_.vector().size()
		);
		boost::iostreams::copy(limiting, parse_body_);
		if (parse_body_.vector().size() < size) return false;
		return parse_varint<packet_id::id_type>(parse_body_).bind([&] (auto id) -> parse_result_type {
			parse_packet_id_.emplace(id, direction_, state_);
			auto serializer = get(map_, *parse_packet_id_);
			if (!serializer) return true;
			auto retr = serializer->parse(parse_body_, parse_pointer_).map([] () noexcept {
				return true;
			});
			auto offset = iostreams::offset(parse_body_, std::ios_base::in);
			assert(offset);
			if (*offset != n) return boost::make_unexpected(
				make_error_code(error::inconsistent_length)
			);
			return retr;
		});
	}
	parse_result_type parse_uncompressed (Source & src) {
		return parse_size_a_.parse(src).bind([&] (auto opt) -> parse_result_type {
			if (!opt) return false;
			//	For some reason GCC 6.3 requires
			//	this->parse_body rather than just
			//	parse_body
			return this->parse_body(src, *opt);
		});
	}
	parse_result_type parse_compressed (Source & src) {
		return parse_size_a_.parse(src).bind([&] (auto opt) -> parse_result_type {
			if (!opt) return false;
			//	TODO: Safe conversion to std::size_t?
			std::size_t size(*opt);
			auto body = iostreams::make_limiting_source(
				boost::ref(src),
				size - parse_body_consumed_
			);
			//	TODO: Update parse_body_consumed_ regardless
			//	of how the method exits
			auto retr = parse_size_b_.parse(body).bind([&] (auto opt) -> parse_result_type {
				if (!opt) {
					return false;
				}
				if (parse_body_compressed_size_ == 0) parse_body_compressed_size_ = body.remaining();
				if (*opt == 0) {
					//	Data is not compressed
					std::size_t body_length(size - parse_size_b_.cached());
					//	Should have been compressed
					if (body_length >= *threshold_) return boost::make_unexpected(
						make_error_code(error::uncompressed)
					);
					//	For some reason GCC 6.3 requires
					//	this->parse_body rather than just
					//	parse_body
					return this->parse_body(body, std::uint32_t(body_length));
				}
				//	Should have been uncompressed
				if (*opt < *threshold_) return boost::make_unexpected(
					make_error_code(error::compressed)
				);
				auto composed = boost::iostreams::compose(
					boost::ref(parse_decompressor_),
					boost::ref(body)
				);
				//	For some reason GCC 6.3 requires
				//	this->parse_body rather than just
				//	parse_body
				return this->parse_body(composed, *opt);
			});
			parse_body_consumed_ = size - body.remaining();
			if ((parse_body_consumed_ == size) && retr && !*retr) return boost::make_unexpected(
				make_error_code(error::end_of_file)
			);
			return retr;
		});
	}
	void parse_reset_if_applicable () noexcept {
		//	If no packet ID has been extracted then
		//	the previous parse has not completed
		if (!parse_packet_id_) return;
		parse_body_.clear();
		parse_size_a_.reset();
		parse_size_b_.reset();
		parse_packet_id_ = nullopt;
		parse_pointer_.reset();
		parse_body_consumed_ = 0;
		parse_body_compressed_size_ = 0;
	}
public:
	/**
	 *	Attempts to parse a \ref packet from a `Source`.
	 *
	 *	\param [in] src
	 *		The `Source` from which bytes shall be drawn.
	 *
	 *	\return
	 *		See \ref parse_result_type.
	 */
	parse_result_type parse (Source & src) {
		parse_reset_if_applicable();
		if (threshold_) return parse_compressed(src);
		return parse_uncompressed(src);
	}
	/**
	 *	Determines whether this object manages a \ref protocol::packet
	 *	or not.
	 *
	 *	It is possible for a call to \ref parse to succeed,
	 *	return \em true, and for this object to not manage
	 *	a \ref protocol::packet. This will be the case if the last
	 *	packet parsed had no associated \ref packet_serializer
	 *	and therefore could not be parsed into a \ref protocol::packet
	 *	object.
	 *
	 *	If this method is called and it is not the case that:
	 *
	 *	-	\ref parse has been invoked at least once
	 *	-	The last call to \ref parse returned \em true
	 *
	 *	the behavior is undefined.
	 *
	 *	\return
	 *		\em true if this object manages a \ref protocol::packet,
	 *		\em false otherwise.
	 */
	bool has_packet () const noexcept {
		assert(parse_packet_id_);
		return bool(parse_pointer_);
	}
	/**
	 *	Retrieves the last \ref protocol::packet parsed.
	 *
	 *	If this method is called and it is not the case
	 *	that:
	 *
	 *	-	\ref parse has been invoked at least once
	 *	-	The last call to \ref parse returned \em true
	 *	-	\ref has_packet returns \em true
	 *
	 *	the behavior is undefined.
	 *
	 *	\return
	 *		A reference to the last \ref protocol::packet
	 *		parsed. This reference does not remain valid past
	 *		the next call to \ref parse.
	 */
	const protocol::packet & packet () const noexcept {
		assert(has_packet());
		return *parse_pointer_;
	}
	/**
	 *	Retrieves a \ref packet_id representing the ID
	 *	of the last packet parsed.
	 *
	 *	If this method is called and it is not the case
	 *	that:
	 *
	 *	-	\ref parse has been invoked at least once
	 *	-	The last call to \ref parse returned \em true
	 *
	 *	the behavior is undefined.
	 *
	 *	\return
	 *		A \ref packet_id object.
	 */
	const packet_id & id () const noexcept {
		assert(parse_packet_id_);
		return *parse_packet_id_;
	}
	/**
	 *	Obtains the uncompressed body of the last packet
	 *	parsed.
	 *
	 *	The "body" of a packet is the bytes which would
	 *	be consumed by \ref packet_serializer::parse
	 *	or produced by \ref packet_serializer::serialize
	 *	prepended by the Varint representation of the packet's
	 *	ID. This notably excludes the length prefix (or
	 *	prefixes in the case of compressed mode).
	 *
	 *	If this method is called and it is not the case
	 *	that:
	 *
	 *	-	\ref parse has been invoked at least once
	 *	-	The last call to \ref parse returned \em true
	 *
	 *	the behavior is undefined.
	 *
	 *	If the lifetime of the stream_serializer ends
	 *	the effect of reading from the returned `Source`
	 *	thereafter is undefined.
	 *
	 *	\sa
	 *		parsed_size, parsed_empty, parsed_compressed,
	 *		parsed_compressed_size
	 *
	 *	\return
	 *		A model of `Source` whose managed character
	 *		sequence is the body of the last parsed
	 *		packet.
	 */
	parsed_source parsed () const {
		assert(parse_packet_id_);
		auto && v = parse_body_.vector();
		return buffer(v.data(), v.size());
	}
	/**
	 *	Obtains the length of the uncompressed body
	 *	of the last packet parsed.
	 *
	 *	If this method is called and it is not the case
	 *	that:
	 *
	 *	-	\ref parse has been invoked at least once
	 *	-	The last call to \ref parse returned \em true
	 *
	 *	the behavior is undefined.
	 *
	 *	\sa
	 *		parsed, parsed_empty, parsed_compressed,
	 *		parsed_compressed_size
	 *
	 *	\return
	 *		The size in characters.
	 */
	std::size_t parsed_size () const noexcept {
		assert(parse_packet_id_);
		return parse_body_.vector().size();
	}
	/**
	 *	Determines whether the last packet parsed had
	 *	an empty body.
	 *
	 *	Equivalent to `parsed_size() == 0`. All restrictions
	 *	on invoking \ref parsed_size apply.
	 *
	 *	\sa
	 *		parsed, parsed_size, parsed_compressed,
	 *		parsed_compressed_size
	 *
	 *	\return
	 *		\em true if empty, \em false otherwise.
	 */
	bool parsed_empty () const noexcept {
		return parsed_size() == 0;
	}
	/**
	 *	Determines if the last packet parsed was
	 *	compressed.
	 *
	 *	Note that just because \ref compressed
	 *	returns \em true and a packet was parsed
	 *	does not mean the last packet was compressed.
	 *	The Minecraft protocol allows for packets
	 *	smaller than the compression threshold to
	 *	be sent uncompressed even when compression
	 *	is enabled.
	 *
	 *	If this method is called and it is not the
	 *	case that:
	 *
	 *	-	\ref parse has been invoked at least once
	 *	-	The last call to \ref parse returned \em true
	 *
	 *	the behavior is undefined.
	 *
	 *	\sa
	 *		parsed, parsed_size, parsed_empty, parsed_compressed_size
	 *
	 *	\return
	 *		\em true if the last packet parsed
	 *		was compressed, \em false otherwise.
	 */
	bool parsed_compressed () const noexcept {
		assert(parse_packet_id_);
		if (!threshold_) return false;
		return parse_size_b_.get() != 0;
	}
	/**
	 *	Determines the size of the compressed body
	 *	of the last packet parsed.
	 *
	 *	If \ref parsed_compressed returns \em false
	 *	and this method is invoked the behavior is
	 *	undefined.
	 *
	 *	\sa
	 *		parsed, parsed_size, parsed_empty, parsed_compressed
	 *
	 *	\return
	 *		The size in characters.
	 */
	std::size_t parsed_compressed_size () const noexcept {
		assert(parsed_compressed());
		return parse_body_compressed_size_;
	}
	/**
	 *	Determines the number of characters cached by
	 *	this object.
	 *
	 *	This is the number of characters extracted by
	 *	the \ref parse method since the last time that
	 *	method returned \em true.
	 *
	 *	\return
	 *		The number of characters.
	 */
	std::size_t cached () const noexcept {
		std::size_t retr(parse_size_a_.cached());
		if (threshold_) return retr + parse_body_consumed_;
		return retr + parse_body_.vector().size();
	}
	/**
	 *	Determines if this object has no cached
	 *	characters.
	 *
	 *	Equivalent to `cached() == 0`.
	 *
	 *	\return
	 *		\em true if this object has no cached
	 *		characters, \em false otherwise.
	 */
	bool empty () const noexcept {
		return cached() == 0;
	}
//
//	Serialize
//
public:
	/**
	 *	A type which models `Source` an instance of which
	 *	is returned by \ref serialized.
	 */
	using serialized_source = buffer;
private:
	//	TODO: Make this able to take a stateful allocator
	using compressor_type = boost::iostreams::zlib_compressor;
	using size_buffer_type = iostreams::char_type_of_t<Sink> [varint_size<size_type>];
	inner_sink_type serialize_body_;
	inner_sink_type serialize_compressed_;
	compressor_type serialize_compressor_;
	bool serialize_is_compressed_;
	void serialize_reset () noexcept {
		serialize_body_.clear();
		serialize_compressed_.clear();
		serialize_is_compressed_ = false;
	}
	void serialize_body (const protocol::packet & p) {
		auto serializer = get(map_, p);
		if (!serializer) throw packet_serializer_not_found(p);
		//	TODO: Check direction/state?
		serialize_varint(serializer->id().id(), serialize_body_);
		serializer->serialize(p, serialize_body_);
	}
	void serialize_uncompressed (const protocol::packet & p, Sink & sink) {
		serialize_body(p);
		size_buffer_type size_buffer;
		buffer out(size_buffer);
		auto size = serialize_body_.vector().size();
		auto size_32 = checked::cast<size_type>(size);
		if (!size_32) {
			std::ostringstream ss;
			ss << "Packet length " << size << " unrepresentable";
			throw unrepresentable_error(ss.str());
		}
		serialize_varint(*size_32, out);
		buffer in(size_buffer, out.written());
		auto full = iostreams::make_proxy_source(
			iostreams::make_concatenating_source(
				in,
				boost::ref(serialize_body_)
			)
		);
		auto proxy_sink = iostreams::make_proxy_sink(boost::ref(sink));
		boost::iostreams::copy(full, proxy_sink);
	}
	void serialize_compressed (const protocol::packet & p, Sink & sink) {
		serialize_body(p);
		serialize_is_compressed_ = serialize_body_.vector().size() >= *threshold_;
		if (serialize_is_compressed_) boost::iostreams::copy(
			iostreams::make_proxy_source(boost::ref(serialize_body_)),
			boost::iostreams::compose(
				boost::ref(serialize_compressor_),
				iostreams::make_proxy_sink(boost::ref(serialize_compressed_))
			)
		);
		size_buffer_type uncompressed_size_buffer;
		buffer uncompressed_size_out(uncompressed_size_buffer);
		optional<size_type> uncompressed_size_32(0);
		if (serialize_is_compressed_) {
			auto uncompressed_size = serialize_body_.vector().size();
			uncompressed_size_32 = checked::cast<size_type>(uncompressed_size);
			if (!uncompressed_size_32) {
				std::ostringstream ss;
				ss << "Compressed data length " << uncompressed_size << " unrepresentable";
				throw unrepresentable_error(ss.str());
			}
		}
		serialize_varint(*uncompressed_size_32, uncompressed_size_out);
		size_buffer_type compressed_size_buffer;
		buffer compressed_size_out(compressed_size_buffer);
		std::size_t compressed_size = (
			serialize_is_compressed_ ? serialize_compressed_ : serialize_body_
		).vector().size() + uncompressed_size_out.written();
		auto compressed_size_32 = checked::cast<size_type>(compressed_size);
		if (!compressed_size_32) {
			std::ostringstream ss;
			ss << "Packet length " << compressed_size << " unrepresentable";
			throw unrepresentable_error(ss.str());
		}
		serialize_varint(*compressed_size_32, compressed_size_out);
		buffer compressed_size_in(compressed_size_buffer, compressed_size_out.written());
		buffer uncompressed_size_in(uncompressed_size_buffer, uncompressed_size_out.written());
		auto full = iostreams::make_proxy_source(
			iostreams::make_concatenating_source(
				compressed_size_in,
				uncompressed_size_in,
				boost::ref(serialize_is_compressed_ ? serialize_compressed_ : serialize_body_)
			)
		);
		auto proxy_sink = iostreams::make_proxy_sink(boost::ref(sink));
		boost::iostreams::copy(full, proxy_sink);
	}
public:
	/**
	 *	Serializes a \ref protocol::packet if possible.
	 *
	 *	If no \ref packet_serializer for \em p could be
	 *	found in the managed \ref packet_serializer_map_type
	 *	then an exception shall be thrown.
	 *
	 *	\param [in] p
	 *		The \ref protocol::packet to serialize.
	 *	\param [in] sink
	 *		The `Sink` into which the serialization of
	 *		\em p shall be written.
	 */
	void serialize (const protocol::packet & p, Sink & sink) {
		serialize_reset();
		if (threshold_) serialize_compressed(p, sink);
		else serialize_uncompressed(p, sink);
	}
	/**
	 *	Obtains a `Source` which manages a character sequence
	 *	which is the body of the last packet serialized.
	 *
	 *	The "body" of a packet is the bytes generated by
	 *	\ref packet_serializer::serialize and consumed by
	 *	\ref packet_serializer::parse in addition to a Varint
	 *	representing the packet's ID.
	 *
	 *	If the lifetime of this object ends the behavior of
	 *	reading from the returned `Source` is undefined thereafter.
	 *
	 *	\return
	 *		A `Source`.
	 */
	serialized_source serialized () const {
		auto && v = serialize_body_.vector();
		return serialized_source(v.data(), v.size());
	}
	/**
	 *	Obtains the number of characters in the sequence
	 *	managed by the `Source` which would be returned by
	 *	\ref serialized.
	 *
	 *	\return
	 *		The number of characters.
	 */
	std::size_t serialized_size () const noexcept {
		return serialize_body_.vector().size();
	}
	/**
	 *	Determines whether the source which would be returned
	 *	by \ref serialized manages an empty sequence of
	 *	characters.
	 *
	 *	Equivalent to `serialized_size() == 0`.
	 *
	 *	\return
	 *		\em true if the managed character sequence would
	 *		be empty, \em false otherwise.
	 */
	bool serialized_empty () const noexcept {
		return serialized_size() == 0;
	}
	/**
	 *	Determines whether the last serialized packet was
	 *	compressed.
	 *
	 *	Note that due to the existence of a compression
	 *	threshold this method is not equivalent to the
	 *	return result of \ref compressed at the time the
	 *	last \ref packet was serialized.
	 *
	 *	\return
	 *		\em true if compressed, \em false otherwise.
	 */
	bool serialized_compressed () const noexcept {
		return serialize_is_compressed_;
	}
	/**
	 *	Determines the number of characters in the compressed
	 *	body of the last serialized packet.
	 *
	 *	The behavior of this method is undefined if it
	 *	is the case that \ref serialized_compressed
	 *	returns \em false.
	 *
	 *	\return
	 *		The number of characters.
	 */
	std::size_t serialized_compressed_size () const noexcept {
		assert(serialized_compressed());
		return serialize_compressed_.vector().size();
	}
//
//	Shared
//
private:
	void check_no_parse_in_progress () const noexcept {
		assert(
			parse_packet_id_ ||
			(
				parse_body_.vector().empty() &&
				parse_size_a_.empty() &&
				parse_size_b_.empty() &&
				(parse_body_consumed_ == 0) &&
				(parse_body_compressed_size_ == 0)
			)
		);
	}
public:
	stream_serializer () = delete;
	stream_serializer (const stream_serializer &) = delete;
	stream_serializer (stream_serializer &&) = delete;
	stream_serializer & operator = (const stream_serializer &) = delete;
	stream_serializer & operator = (stream_serializer &&) = delete;
	/**
	 *	Creates a new stream_serializer.
	 *
	 *	Newly-constructed stream_serializer objects do not
	 *	have compression enabled.
	 *
	 *	\param [in] map
	 *		A map of \ref packet_serializer objects which
	 *		the newly-constructed stream_serializer shall
	 *		use to parse and serialize packets. The
	 *		`get_allocator` method of this object shall be
	 *		used to obtain `Allocator` objects as needed
	 *		to allocate memory throughout the newly-constructed
	 *		stream_serializer object's lifetime.
	 *	\param [in] d
	 *		The protocol direction for which the newly-constructed
	 *		serializer shall initially parse packets.
	 *	\param [in] s
	 *		The protocol state for which the newly-constructed
	 *		serializer shall initially parse packets. Defaults
	 *		to \ref protocol::state::handshaking as this is the
	 *		initial state of the Minecraft protocol.
	 *	\param [in] zlib
	 *		Parameters which shall be used for zlib compression
	 *		and decompression. Defaults to a default constructed
	 *		`boost::iostreams::zlib_params` which simply accepts
	 *		the default settings.
	 */
	stream_serializer (
		packet_serializer_map_type map,
		protocol::direction d,
		protocol::state s = state::handshaking,
		const boost::iostreams::zlib_params & zlib = boost::iostreams::zlib_params{} 
	)	:	map_(std::move(map)),
			direction_(d),
			state_(s),
			parse_body_(inner_source_vector_type(inner_source_allocator_type(map_.get_allocator()))),
			parse_decompressor_(zlib),
			parse_body_consumed_(0),
			parse_body_compressed_size_(0),
			serialize_body_(inner_sink_vector_type(inner_sink_allocator_type(map_.get_allocator()))),
			serialize_compressed_(inner_sink_vector_type(inner_sink_allocator_type(map_.get_allocator()))),
			serialize_compressor_(zlib),
			serialize_is_compressed_(false)
	{	}
	/**
	 *	Enables compression and sets the compression
	 *	threshold.
	 *
	 *	If compression is already enabled the threshold
	 *	shall be set.
	 *
	 *	All packets equal to or greater than the given
	 *	threshold shall be compressed (or in the case of
	 *	parsing shall be expected to be compressed).
	 *
	 *	Invoking this method after a call to \ref parse
	 *	has returned \em false or a `std::error_code`
	 *	causes an subsequent invocations of \ref parse
	 *	to have undefined behavior.
	 *
	 *	\param [in] threshold
	 *		The threshold.
	 */
	void enable_compression (std::size_t threshold) noexcept {
		check_no_parse_in_progress();
		threshold_ = threshold;
	}
	/**
	 *	Disables compression.
	 *
	 *	If compression is not enabled nothing happens.
	 *
	 *	Invoking this method after a call to \ref parse
	 *	has returned \em false or a `std::error_code`
	 *	causes an subsequent invocations of \ref parse
	 *	to have undefined behavior.
	 */
	void disable_compression () noexcept {
		check_no_parse_in_progress();
		threshold_ = nullopt;
	}
	/**
	 *	Determines whether compression is enabled or
	 *	not.
	 *
	 *	\return
	 *		\em true if compression is enabled, \em false
	 *		otherwise.
	 */
	bool compressed () const noexcept {
		return bool(threshold_);
	}
	/**
	 *	Retrieves the compression threshold.
	 *
	 *	If \ref compressed returns \em false this method's
	 *	behavior is undefined.
	 *
	 *	\return
	 *		The compression threshold.
	 */
	std::size_t compression_threshold () const noexcept {
		assert(compressed());
		return *threshold_;
	}
	/**
	 *	Retrieves the protocol direction for which the object
	 *	is parsing packets.
	 *
	 *	\return
	 *		The current protocol direction.
	 */
	protocol::direction direction () const noexcept {
		return direction_;
	}
	/**
	 *	Sets the protocol direction for which the object is
	 *	parsing packets.
	 *
	 *	If this method is invoked and the last call to
	 *	\ref parse did not return \em true the behavior is
	 *	undefined.
	 *
	 *	\param [in] d
	 *		The new protocol direction.
	 */
	void direction (protocol::direction d) noexcept {
		check_no_parse_in_progress();
		direction_ = d;
	}
	/**
	 *	Retrieves the protocol state for which the object
	 *	is parsing packets.
	 *
	 *	\return
	 *		The current protocol state.
	 */
	protocol::state state () const noexcept {
		return state_;
	}
	/**
	 *	Sets the protocol state for which the object is
	 *	parsing packets.
	 *
	 *	If this method is invoked and the last call to
	 *	\ref parse did not return \em true the behavior
	 *	is undefined.
	 *
	 *	\param [in] s
	 *		The new protocol state.
	 */
	void state (protocol::state s) noexcept {
		check_no_parse_in_progress();
		state_ = s;
	}
};

}
}
