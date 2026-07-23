#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_HXX
#define MGMAKE_DISCOVERY_TOOL_HXX

#include "../meta/static_string.hxx"
#include "../meta/type_builder.hxx"
#include "../meta/type_map.hxx"

namespace mgmake::discovery {
	template<typename storage_t = meta::type_map<>>
	struct tool_impl : public meta::type_builder<tool_impl, storage_t> {
		using builder_type = meta::type_builder<tool_impl, storage_t>;

		template<meta::static_string logical_v>
		[[nodiscard]] static consteval auto logical() {
			return builder_type::template set_str<"logical", logical_v>();
		}
		static consteval auto logical() {
			return builder_type::template get_str<"logical">();
		}

		template<auto role_v>
		[[nodiscard]] static consteval auto role() {
			return builder_type::template set_value<"role", role_v>();
		}
		static consteval auto role() {
			return builder_type::template get_value_or<"role", nullptr>();
		}
	};
	static constexpr auto tool = tool_impl<>{};
}

#endif // MGMAKE_DISCOVERY_TOOL_HXX
