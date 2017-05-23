#include <boost/expected/expected.hpp>
#include <boost/system/error_code.hpp>
#include <mcpp/checked.hpp>
#include <mcpp/optional.hpp>
#include <mcpp/variant.hpp>
#include <mcpp/yggdrasil/agent.hpp>
#include <mcpp/yggdrasil/authenticate.hpp>
#include <mcpp/yggdrasil/error.hpp>
#include <mcpp/yggdrasil/json.hpp>
#include <mcpp/yggdrasil/profile.hpp>
#include <mcpp/yggdrasil/refresh.hpp>
#include <mcpp/yggdrasil/user.hpp>
#include <mcpp/yggdrasil/validate.hpp>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace mcpp {
namespace yggdrasil {

const std::string & to_string (from_json_error c) {
	static const std::string incorrect_type("Incorrect type");
	static const std::string overflow("Overflow");
	static const std::string unexpected_key("Unexpected key");
	static const std::string invalid_json("Invalid JSON");
	static const std::string incomplete("Expected data missing");
	static const std::string duplicate("Duplicate key");
	switch (c) {
	case from_json_error::incorrect_type:
		return incorrect_type;
	case from_json_error::overflow:
		return overflow;
	case from_json_error::unexpected_key:
		return unexpected_key;
	case from_json_error::invalid_json:
		return invalid_json;
	case from_json_error::incomplete:
		return incomplete;
	case from_json_error::duplicate_key:
		return duplicate;
	default:
		break;
	}
	throw std::logic_error("Unrecognized error code");
}

namespace {

class from_json_error_category_impl final : public std::error_category, public boost::system::error_category {
public:
	virtual const char * name () const noexcept override {
		return "Yggdrasil JSON Parse";
	}
	virtual std::string message (int condition) const override {
		from_json_error e = static_cast<from_json_error>(condition);
		return to_string(e);
	}
};

}

static const from_json_error_category_impl & get_from_json_error_category () {
	static const from_json_error_category_impl retr;
	return retr;
}

const std::error_category & from_json_error_category () {
	return get_from_json_error_category();
}

namespace detail {

const boost::system::error_category & from_json_boost_error_category () {
	return get_from_json_error_category();
}

}

std::error_code make_error_code (from_json_error e) noexcept {
	return std::error_code(
		static_cast<int>(e),
		from_json_error_category()
	);
}

std::error_condition make_error_condition (from_json_error e) noexcept {
	return std::error_condition(
		static_cast<int>(e),
		from_json_error_category()
	);
}

static void check (bool val) {
	if (!val) throw std::runtime_error("RapidJSON error");
}

template <std::size_t N, typename T>
static void key (const char (& arr) [N], T & writer) {
	constexpr auto size = N - 1;
	check(writer.Key(arr, size));
}

template <typename T>
static void to_json (const std::string & str, T & writer) {
	check(writer.String(str.c_str(), str.size()));
}

template <typename F>
static void to_json_wrapper (F func, std::ostream & os) {
	rapidjson::BasicOStreamWrapper<std::ostream> stream(os);
	//	TODO: Support pretty printing?
	rapidjson::Writer<decltype(stream)> writer(stream);
	check(writer.StartObject());
	func(writer);
	check(writer.EndObject());
	writer.Flush();
}

template <typename T>
static void to_json (const agent & a, T & writer) {
	check(writer.StartObject());
	key("name", writer);
	to_json(a.name, writer);
	key("version", writer);
	check(writer.Uint(a.version));
	check(writer.EndObject());
}

template <typename T>
static void to_json (const profile & p, T & writer) {
	check(writer.StartObject());
	key("id", writer);
	to_json(p.id, writer);
	key("name", writer);
	to_json(p.name, writer);
	key("legacy", writer);
	check(writer.Bool(p.legacy));
	check(writer.EndObject());
}

template <typename T>
static void to_json (const user & u, T & writer) {
	check(writer.StartObject());
	key("id", writer);
	to_json(u.id, writer);
	key("properties", writer);
	check(writer.StartArray());
	for (auto && pair : u.properties) {
		check(writer.StartObject());
		key("name", writer);
		to_json(pair.first, writer);
		key("value", writer);
		to_json(pair.second, writer);
		check(writer.EndObject());
	}
	check(writer.EndArray());
	check(writer.EndObject());
}

void to_json (const authenticate_request & request, std::ostream & os) {
	to_json_wrapper([&] (auto & writer) {
		if (request.agent) {
			key("agent", writer);
			to_json(*request.agent, writer);
		}
		key("username", writer);
		to_json(request.username, writer);
		key("password", writer);
		to_json(request.password, writer);
		if (request.client_token) {
			key("clientToken", writer);
			to_json(*request.client_token, writer);
		}
		key("requestUser", writer);
		check(writer.Bool(request.request_user));
	}, os);
}

static bool strnequal (const char * a, const char * b, std::size_t count) noexcept {
	return std::strncmp(a, b, count) == 0;
}

namespace {

class handler_base {
private:
	optional<from_json_error> error_;
protected:
	void error (from_json_error c) noexcept {
		if (!error_) error_ = c;
	}
private:
	bool impl () noexcept {
		error(from_json_error::incorrect_type);
		return false;
	}
public:
	using Ch = char;
	bool Null () noexcept {
		return impl();
	}
	bool Bool (bool) noexcept {
		return impl();
	}
	bool Int (int) noexcept {
		return impl();
	}
	bool Uint (unsigned) noexcept {
		return impl();
	}
	bool Int64 (std::int64_t) noexcept {
		return impl();
	}
	bool Uint64 (std::uint64_t) noexcept {
		return impl();
	}
	bool Double (double) noexcept {
		return impl();
	}
	bool RawNumber (const char *, std::size_t, bool) noexcept {
		return impl();
	}
	bool String (const char *, std::size_t, bool) noexcept {
		return impl();
	}
	bool StartObject () noexcept {
		return impl();
	}
	bool Key (const char *, std::size_t, bool) noexcept {
		return impl();
	}
	bool EndObject (std::size_t) noexcept {
		return impl();
	}
	bool StartArray () noexcept {
		return impl();
	}
	bool EndArray (std::size_t) noexcept {
		return impl();
	}
	explicit operator bool () const noexcept {
		return !bool(error_);
	}
	from_json_error error () const noexcept {
		assert(error_);
		return *error_;
	}
};

template <typename... Handlers>
class parser_handler : public handler_base {
private:
	using base = handler_base;
	optional<variant<Handlers...>> ps_;
	template <typename Handler>
	void reset (const Handler & h) noexcept {
		if (h.done()) ps_ = nullopt;
	}
	template <typename F>
	bool visit (F func) {
		return mcpp::visit([&] (auto && h) {
			auto retr = func(h);
			this->reset(h);
			return retr;
		}, *ps_);
	}
protected:
	template <typename T, typename... Args>
	T & emplace (Args &&... args) noexcept(std::is_nothrow_constructible<T, Args &&...>::value) {
		ps_.emplace(
			in_place_type<T>,
			std::forward<Args>(args)...
		);
		//	This is necessary since apparently as of
		//	GCC 6.3.0 std::experimental::optional::emplace
		//	doesn't return a reference as it should
		return get<T>(*ps_);
	}
	void error (from_json_error c) noexcept {
		ps_ = nullopt;
		base::error(c);
	}
public:
	bool Null () {
		if (!ps_) return base::Null();
		return visit([] (auto && h) {	return h.Null();	});
	}
	bool Bool (bool b) {
		if (!ps_) return base::Bool(b);
		return visit([&] (auto && h) {	return h.Bool(b);	});
	}
	bool Int (int i) {
		if (!ps_) return base::Int(i);
		return visit([&] (auto && h) {	return h.Int(i);	});
	}
	bool Uint (unsigned u) {
		if (!ps_) return base::Uint(u);
		return visit([&] (auto && h) {	return h.Uint(u);	});
	}
	bool Int64 (std::int64_t i64) {
		if (!ps_) return base::Int64(i64);
		return visit([&] (auto && h) {	return h.Int64(i64);	});
	}
	bool Uint64 (std::uint64_t u64) {
		if (!ps_) return base::Uint64(u64);
		return visit([&] (auto && h) {	return h.Uint64(u64);	});
	}
	bool Double (double d) {
		if (!ps_) return base::Double(d);
		return visit([&] (auto && h) {	return h.Double(d);	});
	}
	bool RawNumber (const char * str, std::size_t length, bool copy) {
		if (!ps_) return base::RawNumber(str, length, copy);
		return visit([&] (auto && h) {	return h.RawNumber(str, length, copy);	});
	}
	bool String (const char * str, std::size_t length, bool copy) {
		if (!ps_) return base::String(str, length, copy);
		return visit([&] (auto && h) {	return h.String(str, length, copy);	});
	}
	bool StartObject () {
		if (!ps_) return base::StartObject();
		return visit([] (auto && h) {	return h.StartObject();	});
	}
	bool Key (const char * str, std::size_t length, bool copy) {
		if (!ps_) return base::Key(str, length, copy);
		return visit([&] (auto && h) {	return h.Key(str, length, copy);	});
	}
	bool EndObject (std::size_t memberCount) {
		if (!ps_) return base::EndObject(memberCount);
		return visit([&] (auto && h) {	return h.EndObject(memberCount);	});
	}
	bool StartArray () {
		if (!ps_) return base::StartArray();
		return visit([] (auto && h) {	return h.StartArray();	});
	}
	bool EndArray (std::size_t memberCount) {
		if (!ps_) return base::EndArray(memberCount);
		return visit([&] (auto && h) {	return h.EndArray(memberCount);	});
	}
	explicit operator bool () const noexcept {
		if (ps_) return mcpp::visit([] (auto && h) noexcept {	return bool(h);	}, *ps_);
		return bool(static_cast<const handler_base &>(*this));
	}
	from_json_error error () const noexcept {
		assert(!*this);
		if (ps_) return mcpp::visit([] (auto && h) noexcept {	return h.error();	}, *ps_);
		return base::error();
	}
	bool done () const noexcept {
		return !bool(ps_);
	}
};

class complete_handler_base : public handler_base {
private:
	bool done_;
protected:
	void complete () noexcept {
		done_ = true;
	}
public:
	complete_handler_base () noexcept : done_(false) {	}
	bool done () const noexcept {
		return done_;
	}
};

template <typename... Handlers>
class object_handler_base : public parser_handler<Handlers...> {
private:
	using base = parser_handler<Handlers...>;
	bool started_;
	bool finished_;
protected:
	using parser = base;
public:
	object_handler_base () noexcept : started_(false), finished_(false) {	}
	bool StartObject () {
		if (finished_) throw std::logic_error("StartObject after matching EndObject");
		if (started_) return base::StartObject();
		started_ = true;
		return true;
	}
	bool EndObject (std::size_t memberCount) {
		if (!started_) throw std::logic_error("EndObject without StartObject");
		if (!base::done()) return base::EndObject(memberCount);
		finished_ = true;
		return true;
	}
	bool done () const noexcept {
		return finished_ && base::done();
	}
};

class string_handler : public complete_handler_base {
private:
	std::string & str_;
public:
	explicit string_handler (std::string & str) noexcept : str_(str) {	}
	bool String (const char * str, std::size_t length, bool) {
		str_.assign(str, length);
		complete();
		return true;
	}
};

class bool_handler : public complete_handler_base {
private:
	bool & b_;
public:
	explicit bool_handler (bool & b) noexcept : b_(b) {	}
	bool Bool (bool b) noexcept {
		b_ = b;
		complete();
		return true;
	}
};

template <typename Integer>
class integer_handler : public complete_handler_base {
private:
	using base = complete_handler_base;
	Integer & i_;
	template <typename Integer2>
	bool assign (Integer2 i) noexcept {
		auto opt = checked::cast<Integer>(i);
		if (!opt) {
			error(from_json_error::overflow);
			return false;
		}
		i_ = *opt;
		complete();
		return true;
	}
public:
	explicit integer_handler (Integer & i) noexcept : i_(i) {	}
	bool Int (int i) noexcept {
		return assign(i);
	}
	bool Uint (unsigned u) noexcept {
		return assign(u);
	}
	bool Int64 (std::int64_t i64) noexcept {
		return assign(i64);
	}
	bool Uint64 (std::uint64_t u64) noexcept {
		return assign(u64);
	}
	//	TODO: Is it necessary to support Double
	//	and/or RawNumber?
};

class agent_handler : public object_handler_base<integer_handler<unsigned>, string_handler> {
private:
	using base = object_handler_base<integer_handler<unsigned>, string_handler>;
	agent & a_;
	bool name_;
	bool version_;
public:
	explicit agent_handler (agent & a) noexcept
		:	a_(a),
			name_(false),
			version_(false)
	{	}
	bool Key (const Ch * str, std::size_t length, bool copy) {
		if (!parser::done()) return base::Key(str, length, copy);
		if (strnequal("name", str, length)) {
			if (name_) {
				error(from_json_error::duplicate_key);
				return false;
			}
			name_ = true;
			emplace<string_handler>(a_.name);
			return true;
		}
		if (strnequal("version", str, length)) {
			if (version_) {
				error(from_json_error::duplicate_key);
				return false;
			}
			version_ = true;
			emplace<integer_handler<unsigned>>(a_.version);
			return true;
		}
		error(from_json_error::unexpected_key);
		return false;
	}
	bool done () const noexcept {
		return name_ && version_ && base::done();
	}
};

class profile_handler : public object_handler_base<string_handler, bool_handler> {
private:
	using base = object_handler_base<string_handler, bool_handler>;
	profile & p_;
	bool id_;
	bool name_;
	bool legacy_;
public:
	explicit profile_handler (profile & p) noexcept
		:	p_(p),
			id_(false),
			name_(false),
			legacy_(false)
	{	}
	bool Key (const Ch * str, std::size_t length, bool copy) {
		if (!parser::done()) return base::Key(str, length, copy);
		if (strnequal("id", str, length)) {
			if (id_) {
				error(from_json_error::duplicate_key);
				return false;
			}
			id_ = true;
			emplace<string_handler>(p_.id);
			return true;
		}
		if (strnequal("name", str, length)) {
			if (name_) {
				error(from_json_error::duplicate_key);
				return false;
			}
			name_ = true;
			emplace<string_handler>(p_.name);
			return true;
		}
		if (strnequal("legacy", str, length)) {
			if (legacy_) {
				error(from_json_error::duplicate_key);
				return false;
			}
			legacy_ = true;
			emplace<bool_handler>(p_.legacy);
			return true;
		}
		error(from_json_error::unexpected_key);
		return false;
	}
	bool done () const noexcept {
		return id_ && name_ && base::done();
	}
};

class profiles_handler : public parser_handler<profile_handler> {
private:
	using base = parser_handler<profile_handler>;
	std::vector<profile> & ps_;
	bool started_;
	bool finished_;
public:
	explicit profiles_handler (std::vector<profile> & ps)
		:	ps_(ps),
			started_(false),
			finished_(false)
	{	}
	bool StartArray () {
		if (finished_) throw std::logic_error("StartArray after matching EndArray");
		if (started_) return base::StartArray();
		started_ = true;
		return true;
	}
	bool EndArray (std::size_t memberCount) {
		if (!started_) throw std::logic_error("EndArray without StartArray");
		if (!base::done()) return base::EndArray(memberCount);
		finished_ = true;
		return true;
	}
	bool StartObject () {
		if (!started_) {
			error(from_json_error::incorrect_type);
			return false;
		}
		if (base::done()) {
			ps_.emplace_back();
			emplace<profile_handler>(ps_.back());
		}
		return base::StartObject();
	}
	bool done () const noexcept {
		return finished_;
	}
};

class property_handler : public object_handler_base<string_handler> {
private:
	using base = object_handler_base<string_handler>;
	//	Differs from user::properties_type::value_type
	//	due to the fact the first element is not const
	using pair_type = std::pair<std::string, std::string>;
	user::properties_type & ps_;
	pair_type kvp_;
	bool name_;
	bool value_;
public:
	explicit property_handler (user::properties_type & ps)
		:	ps_(ps),
			name_(false),
			value_(false)
	{	}
	bool done () const noexcept {
		return name_ && value_ && base::done();
	}
	bool Key (const Ch * str, std::size_t length, bool copy) {
		if (!parser::done()) return base::Key(str, length, copy);
		if (strnequal("name", str, length)) {
			if (name_) {
				error(from_json_error::duplicate_key);
				return false;
			}
			name_ = true;
			emplace<string_handler>(kvp_.first);
			return true;
		}
		if (strnequal("value", str, length)) {
			if (value_) {
				error(from_json_error::duplicate_key);
				return false;
			}
			value_ = true;
			emplace<string_handler>(kvp_.second);
			return true;
		}
		error(from_json_error::unexpected_key);
		return true;
	}
	bool EndObject (std::size_t memberCount) {
		if (!base::EndObject(memberCount)) return false;
		if (done()) {
			pair_type local;
			using std::swap;
			swap(local, kvp_);
			auto pair = ps_.emplace(std::move(local));
			if (!pair.second) {
				error(from_json_error::duplicate_key);
				return false;
			}
		}
		return true;
	}
};

class properties_handler : public parser_handler<property_handler> {
private:
	using base = parser_handler<property_handler>;
	user::properties_type & ps_;
	bool started_;
	bool finished_;
public:
	explicit properties_handler (user::properties_type & ps) noexcept
		:	ps_(ps),
			started_(false),
			finished_(false)
	{	}
	bool StartArray () {
		if (finished_) throw std::logic_error("StartArray after matching EndArray");
		if (started_) return base::StartArray();
		started_ = true;
		return true;
	}
	bool EndArray (std::size_t memberCount) {
		if (!started_) throw std::logic_error("EndArray without StartArray");
		if (!base::done()) return base::EndArray(memberCount);
		finished_ = true;
		return true;
	}
	bool StartObject () {
		if (base::done()) emplace<property_handler>(ps_);
		return base::StartObject();
	}
	bool done () const noexcept {
		return finished_;
	}
};

class user_handler : public object_handler_base<string_handler, properties_handler> {
private:
	using base = object_handler_base<string_handler, properties_handler>;
	user & u_;
	bool id_;
	bool properties_;
public:
	explicit user_handler (user & u) noexcept
		:	u_(u),
			id_(false),
			properties_(false)
	{	}
	bool done () const noexcept {
		return id_ && properties_ && base::done();
	}
	bool Key (const Ch * str, std::size_t length, bool copy) {
		if (!parser::done()) return base::Key(str, length, copy);
		if (strnequal("id", str, length)) {
			if (id_) {
				error(from_json_error::duplicate_key);
				return false;
			}
			id_ = true;
			emplace<string_handler>(u_.id);
			return true;
		}
		if (strnequal("properties", str, length)) {
			if (properties_) {
				error(from_json_error::duplicate_key);
				return false;
			}
			properties_ = true;
			emplace<properties_handler>(u_.properties);
			return true;
		}
		error(from_json_error::unexpected_key);
		return false;
	}
};

}

template <typename Handler>
from_json_result_type<decltype(std::declval<Handler>().get())> from_json_impl (std::istream & is, Handler & h) {
	rapidjson::BasicIStreamWrapper<std::istream> stream(is);
	rapidjson::Reader reader;
	auto parse_error = reader.Parse(stream, h);
	if (parse_error.IsError()) return boost::make_unexpected(
		make_error_code(
			h ? from_json_error::invalid_json : h.error()
		)
	);
	if (!h.done()) return boost::make_unexpected(
		make_error_code(from_json_error::incomplete)
	);
	return h.get();
}

template <>
from_json_result_type<authenticate_request> from_json<authenticate_request> (std::istream & is) {
	class handler : public object_handler_base<string_handler, bool_handler, agent_handler> {
	private:
		using base = object_handler_base<string_handler, bool_handler, agent_handler>;
		authenticate_request request_;
		bool agent_;
		bool username_;
		bool password_;
		bool client_token_;
		bool request_user_;
	public:
		handler ()
			:	agent_(false),
				username_(false),
				password_(false),
				client_token_(false),
				request_user_(false)
		{	}
		authenticate_request get () {
			return std::move(request_);
		}
		bool done () const noexcept {
			return username_ && password_ && base::done();
		}
		bool Key (const Ch * str, std::size_t length, bool copy) {
			if (!parser::done()) return base::Key(str, length, copy);
			if (strnequal("agent", str, length)) {
				if (agent_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				agent_ = true;
				request_.agent.emplace();
				emplace<agent_handler>(*request_.agent);
				return true;
			}
			if (strnequal("username", str, length)) {
				if (username_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				username_ = true;
				emplace<string_handler>(request_.username);
				return true;
			}
			if (strnequal("password", str, length)) {
				if (password_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				password_ = true;
				emplace<string_handler>(request_.password);
				return true;
			}
			if (strnequal("clientToken", str, length)) {
				if (client_token_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				client_token_ = true;
				request_.client_token.emplace();
				emplace<string_handler>(*request_.client_token);
				return true;
			}
			if (strnequal("requestUser", str, length)) {
				if (request_user_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				request_user_ = true;
				emplace<bool_handler>(request_.request_user);
				return true;
			}
			error(from_json_error::unexpected_key);
			return false;
		}
	};
	handler h;
	return from_json_impl(is, h);
}

void to_json (const authenticate_response & response, std::ostream & os) {
	to_json_wrapper([&] (auto & writer) {
		key("accessToken", writer);
		to_json(response.access_token, writer);
		key("clientToken", writer);
		to_json(response.client_token, writer);
		if (response.available_profiles) {
			key("availableProfiles", writer);
			check(writer.StartArray());
			for (auto && profile : *response.available_profiles) to_json(profile, writer);
			check(writer.EndArray());
		}
		if (response.selected_profile) {
			key("selectedProfile", writer);
			to_json(*response.selected_profile, writer);
		}
		if (response.user) {
			key("user", writer);
			to_json(*response.user, writer);
		}
	}, os);
}

template <>
from_json_result_type<authenticate_response> from_json<authenticate_response> (std::istream & is) {
	class handler : public object_handler_base<
		string_handler,
		profile_handler,
		profiles_handler,
		user_handler
	> {
	private:
		using base = object_handler_base<
			string_handler,
			profile_handler,
			profiles_handler,
			user_handler
		>;
		authenticate_response res_;
		bool access_token_;
		bool client_token_;
		bool available_profiles_;
		bool selected_profile_;
		bool user_;
	public:
		handler ()
			:	access_token_(false),
				client_token_(false),
				available_profiles_(false),
				selected_profile_(false),
				user_(false)
		{	}
		bool Key (const Ch * str, std::size_t length, bool copy) {
			if (!parser::done()) return base::Key(str, length, copy);
			if (strnequal("accessToken", str, length)) {
				if (access_token_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				access_token_ = true;
				emplace<string_handler>(res_.access_token);
				return true;
			}
			if (strnequal("clientToken", str, length)) {
				if (client_token_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				client_token_ = true;
				emplace<string_handler>(res_.client_token);
				return true;
			}
			if (strnequal("availableProfiles", str, length)) {
				if (available_profiles_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				available_profiles_ = true;
				res_.available_profiles.emplace();
				emplace<profiles_handler>(*res_.available_profiles);
				return true;
			}
			if (strnequal("selectedProfile", str, length)) {
				if (selected_profile_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				selected_profile_ = true;
				res_.selected_profile.emplace();
				emplace<profile_handler>(*res_.selected_profile);
				return true;
			}
			if (strnequal("user", str, length)) {
				if (user_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				user_ = true;
				res_.user.emplace();
				emplace<user_handler>(*res_.user);
				return true;
			}
			error(from_json_error::unexpected_key);
			return false;
		}
		bool done () const noexcept {
			return access_token_ && client_token_ && base::done();
		}
		authenticate_response get () {
			return std::move(res_);
		}
	};
	handler h;
	return from_json_impl(is, h);
}

void to_json (const refresh_request & request, std::ostream & os) {
	to_json_wrapper([&] (auto & writer) {
		key("accessToken", writer);
		to_json(request.access_token, writer);
		key("clientToken", writer);
		to_json(request.client_token, writer);
		if (request.selected_profile) {
			key("selectedProfile", writer);
			to_json(*request.selected_profile, writer);
		}
		key("requestUser", writer);
		check(writer.Bool(request.request_user));
	}, os);	
}

template <>
from_json_result_type<refresh_request> from_json<refresh_request> (std::istream & is) {
	class handler : public object_handler_base<
		string_handler,
		bool_handler,
		profile_handler
	> {
	private:
		using base = object_handler_base<
			string_handler,
			bool_handler,
			profile_handler
		>;
		refresh_request req_;
		bool access_token_;
		bool client_token_;
		bool selected_profile_;
		bool request_user_;
	public:
		handler ()
			:	access_token_(false),
				client_token_(false),
				selected_profile_(false),
				request_user_(false)
		{	}
		refresh_request get () {
			return std::move(req_);
		}
		bool done () const noexcept {
			return access_token_ && client_token_ && base::done();
		}
		bool Key (const Ch * str, std::size_t length, bool copy) {
			if (!parser::done()) return base::Key(str, length, copy);
			if (strnequal("accessToken", str, length)) {
				if (access_token_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				access_token_ = true;
				emplace<string_handler>(req_.access_token);
				return true;
			}
			if (strnequal("clientToken", str, length)) {
				if (client_token_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				client_token_ = true;
				emplace<string_handler>(req_.client_token);
				return true;
			}
			if (strnequal("selectedProfile", str, length)) {
				if (selected_profile_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				selected_profile_ = true;
				req_.selected_profile.emplace();
				emplace<profile_handler>(*req_.selected_profile);
				return true;
			}
			if (strnequal("requestUser", str, length)) {
				if (request_user_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				request_user_ = true;
				emplace<bool_handler>(req_.request_user);
				return true;
			}
			error(from_json_error::unexpected_key);
			return false;
		}
	};
	handler h;
	return from_json_impl(is, h);
}

void to_json (const refresh_response & response, std::ostream & os) {
	to_json_wrapper([&] (auto & writer) {
		key("accessToken", writer);
		to_json(response.access_token, writer);
		key("clientToken", writer);
		to_json(response.client_token, writer);
		if (response.selected_profile) {
			key("selectedProfile", writer);
			to_json(*response.selected_profile, writer);
		}
		if (response.user) {
			key("user", writer);
			to_json(*response.user, writer);
		}
	}, os);
}

template <>
from_json_result_type<refresh_response> from_json<refresh_response> (std::istream & is) {
	class handler : public object_handler_base<
		string_handler,
		profile_handler,
		user_handler
	> {
	private:
		using base = object_handler_base<
			string_handler,
			profile_handler,
			user_handler
		>;
		refresh_response res_;
		bool access_token_;
		bool client_token_;
		bool selected_profile_;
		bool user_;
	public:
		handler ()
			:	access_token_(false),
				client_token_(false),
				selected_profile_(false),
				user_(false)
		{	}
		refresh_response get () {
			return std::move(res_);
		}
		bool done () const noexcept {
			return access_token_ && client_token_ && base::done();
		}
		bool Key (const Ch * str, std::size_t length, bool copy) {
			if (!parser::done()) return base::Key(str, length, copy);
			if (strnequal("accessToken", str, length)) {
				if (access_token_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				access_token_ = true;
				emplace<string_handler>(res_.access_token);
				return true;
			}
			if (strnequal("clientToken", str, length)) {
				if (client_token_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				client_token_ = true;
				emplace<string_handler>(res_.client_token);
				return true;
			}
			if (strnequal("selectedProfile", str, length)) {
				if (selected_profile_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				selected_profile_ = true;
				res_.selected_profile.emplace();
				emplace<profile_handler>(*res_.selected_profile);
				return true;
			}
			if (strnequal("user", str, length)) {
				if (user_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				user_ = true;
				res_.user.emplace();
				emplace<user_handler>(*res_.user);
				return true;
			}
			error(from_json_error::unexpected_key);
			return false;
		}
	};
	handler h;
	return from_json_impl(is, h);
}

void to_json (const api_error & e, std::ostream & os) {
	to_json_wrapper([&] (auto & writer) {
		key("error", writer);
		to_json(e.error, writer);
		key("errorMessage", writer);
		to_json(e.error_message, writer);
		if (e.cause) {
			key("cause", writer);
			to_json(*e.cause, writer);
		}
	}, os);
}

template <>
from_json_result_type<api_error> from_json<api_error> (std::istream & is) {
	class handler : public object_handler_base<string_handler> {
	private:
		using base = object_handler_base<string_handler>;
		api_error e_;
		bool error_;
		bool error_message_;
		bool cause_;
	public:
		handler ()
			:	error_(false),
				error_message_(false),
				cause_(false)
		{	}
		api_error get () {
			return std::move(e_);
		}
		bool done () const noexcept {
			return error_ && error_message_ && base::done();
		}
		bool Key (const Ch * str, std::size_t length, bool copy) {
			if (!parser::done()) return base::Key(str, length, copy);
			if (strnequal("error", str, length)) {
				if (error_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				error_ = true;
				emplace<string_handler>(e_.error);
				return true;
			}
			if (strnequal("errorMessage", str, length)) {
				if (error_message_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				error_message_ = true;
				emplace<string_handler>(e_.error_message);
				return true;
			}
			if (strnequal("cause", str, length)) {
				if (cause_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				cause_ = true;
				e_.cause.emplace();
				emplace<string_handler>(*e_.cause);
				return true;
			}
			error(from_json_error::unexpected_key);
			return false;
		}
	};
	handler h;
	return from_json_impl(is, h);
}

void to_json (const validate_request & request, std::ostream & os) {
	to_json_wrapper([&] (auto & writer) {
		key("accessToken", writer);
		to_json(request.access_token, writer);
		if (request.client_token) {
			key("clientToken", writer);
			to_json(*request.client_token, writer);
		}
	}, os);
}

template <>
from_json_result_type<validate_request> from_json<validate_request> (std::istream & is) {
	class handler : public object_handler_base<string_handler> {
	private:
		using base = object_handler_base<string_handler>;
		validate_request req_;
		bool access_token_;
		bool client_token_;
	public:
		handler ()
			:	access_token_(false),
				client_token_(false)
		{	}
		validate_request get () {
			return std::move(req_);
		}
		bool done () const noexcept {
			return access_token_ && base::done();
		}
		bool Key (const Ch * str, std::size_t length, bool copy) {
			if (!parser::done()) return base::Key(str, length, copy);
			if (strnequal("accessToken", str, length)) {
				if (access_token_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				access_token_ = true;
				emplace<string_handler>(req_.access_token);
				return true;
			}
			if (strnequal("clientToken", str, length)) {
				if (client_token_) {
					error(from_json_error::duplicate_key);
					return false;
				}
				client_token_ = true;
				req_.client_token.emplace();
				emplace<string_handler>(*req_.client_token);
				return true;
			}
			error(from_json_error::unexpected_key);
			return false;
		}
	};
	handler h;
	return from_json_impl(is, h);
}

}
}
