#include <mcpp/iostreams/traits.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/char_traits.hpp>
#include <boost/iostreams/traits.hpp>
#include <ostream>
#include <type_traits>

namespace mcpp {
namespace iostreams {
namespace tests {
namespace {

static_assert(std::is_same<char_type_of_t<std::ostream>, char>::value, "Wrong char_type_of_t");

static_assert(std::is_same<int_type_of_t<std::ostream>, std::ostream::int_type>::value, "Wrong int_type_of_t");

static_assert(std::is_same<category_of_t<std::ostream>, boost::iostreams::category_of<std::ostream>::type>::value, "Wrong category_of_t");

static_assert(std::is_same<traits_of_t<std::ostream>, boost::iostreams::char_traits<char>>::value, "Wrong traits_of_t");

static_assert(in_category_v<std::ostream, boost::iostreams::device_tag>, "Wrong in_category_v");
static_assert(!in_category_v<std::ostream, boost::iostreams::peekable_tag>, "Wrong in_category_v");

static_assert(std::is_same<in_category_t<std::ostream, boost::iostreams::device_tag>, std::true_type>::value, "Wrong in_category_t");
static_assert(std::is_same<in_category_t<std::ostream, boost::iostreams::peekable_tag>, std::false_type>::value, "Wrong in_category_t");

static_assert(!in_any_category_v<std::ostream>, "Wrong in_any_category_v (empty)");
static_assert(in_any_category_v<std::ostream, boost::iostreams::device_tag>, "Wrong in_any_category_v (single)");
static_assert(!in_any_category_v<std::ostream, boost::iostreams::peekable_tag>, "Wrong in_any_category_v (single)");
static_assert(in_any_category_v<std::ostream, boost::iostreams::peekable_tag, boost::iostreams::device_tag>, "Wrong in_any_category_v (multiple)");
static_assert(!in_any_category_v<std::ostream, boost::iostreams::peekable_tag, boost::iostreams::direct_tag>, "Wrong in_any_category_v (multiple)");

static_assert(std::is_same<in_any_category_t<std::ostream>, std::false_type>::value, "Wrong in_any_category_t (empty)");
static_assert(std::is_same<in_any_category_t<std::ostream, boost::iostreams::device_tag>, std::true_type>::value, "Wrong in_any_category_t (single)");
static_assert(std::is_same<in_any_category_t<std::ostream, boost::iostreams::peekable_tag>, std::false_type>::value, "Wrong in_any_category_t (single)");
static_assert(std::is_same<in_any_category_t<std::ostream, boost::iostreams::device_tag, boost::iostreams::peekable_tag>, std::true_type>::value, "Wrong in_any_category_t (multiple)");
static_assert(std::is_same<in_any_category_t<std::ostream, boost::iostreams::peekable_tag, boost::iostreams::direct_tag>, std::false_type>::value, "Wrong in_any_category_t (multiple)");

}
}
}
}
