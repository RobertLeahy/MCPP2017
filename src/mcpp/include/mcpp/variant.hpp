/**
 *	\file
 */

#pragma once

#include <mpark/variant.hpp>

namespace mcpp {

using mpark::bad_variant_access;
using mpark::get;
using mpark::get_if;
using mpark::holds_alternative;
using mpark::in_place_index;
using mpark::in_place_index_t;
using mpark::in_place_type;
using mpark::in_place_type_t;
using mpark::monostate;
using mpark::variant;
using mpark::variant_alternative;
using mpark::variant_alternative_t;
using mpark::variant_npos;
using mpark::variant_size;
using mpark::variant_size_v;
using mpark::visit;

}
