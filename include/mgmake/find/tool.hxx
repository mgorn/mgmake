#pragma once

#ifndef MGMAKE_FIND_TOOL_HXX
#define MGMAKE_FIND_TOOL_HXX

#include "../cli/option.hxx"
#include "../meta/static_string.hxx"
#include "../meta/type_builder.hxx"
#include "../meta/value_list.hxx"
#include "../meta/type_map.hxx"

namespace mgmake::find {
	template<typename storage_t = meta::type_map<>>
	struct tool_impl : public meta::type_builder<tool_impl, storage_t>, public meta::named<tool_impl<storage_t>> {
		using builder_type = meta::type_builder<tool_impl, storage_t>;

		// Logical name, for the override option ("cc", "cxx")
		template<meta::static_string logical_v>
		[[nodiscard]] static consteval auto logical() {
			return builder_type::template set_str<"logical", logical_v>();
		}
		static consteval auto logical() {
			return builder_type::template get_str<"logical">();
		}

		// Environment variable override ("MGMK_CC", "MGMK_CXX", etc)
		template<meta::static_string env_v>
		[[nodiscard]] static consteval auto env() {
			return builder_type::template set_str<"env", env_v>();
		}
		static consteval auto env() {
			return builder_type::template get_str<"env">();
		}

		[[nodiscard]] static consteval auto option() {
			constexpr auto description_v = meta::static_string{ "Override the " } + builder_type::template get_str<"name">() + meta::static_string{ " tool" };
			return cli::option
				.name<logical()>()
				.template description<description_v>()
				.template parse<logical(), std::filesystem::path>();
		}
		
		using option_type = decltype(option());
	};
	static constexpr auto tool = tool_impl<>{};
}

#endif // MGMAKE_FIND_TOOL_HXX