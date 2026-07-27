#pragma once

#ifndef MGMAKE_TOOL_TOOL_HXX
#define MGMAKE_TOOL_TOOL_HXX

#include "../cli/option.hxx"
#include "../meta/static_string.hxx"
#include "../meta/type_builder.hxx"
#include "../meta/value_list.hxx"
#include "../meta/type_map.hxx"

namespace mgmake::tool {
	template<typename storage_t = meta::type_map<>>
	struct tool_impl : public meta::type_builder<tool_impl, storage_t>, public meta::named<tool_impl<storage_t>> {
		using builder_type = meta::type_builder<tool_impl, storage_t>;

		// CLI name, for the override option ("cc", "cxx")
		template<meta::static_string cli_v>
		[[nodiscard]] static consteval auto cli() {
			return builder_type::template set_str<"cli", cli_v>();
		}
		static consteval auto cli() {
			return builder_type::template get_str<"cli">();
		}

		// Environment variable override ("MGMK_CC", "MGMK_CXX", etc)
		template<meta::static_string env_v>
		[[nodiscard]] static consteval auto env() {
			return builder_type::template set_str<"env", env_v>();
		}
		static consteval auto env() {
			return builder_type::template get_str<"env">();
		}

		// Default executable name ("cxx_tool" -> "c++")
		template<meta::static_string exe_v>
		[[nodiscard]] static consteval auto exe() {
			return builder_type::template set_str<"exe", exe_v>();
		}
		static consteval auto exe() {
			return builder_type::template get_str<"exe">();
		}

		[[nodiscard]] static consteval auto option() {
			constexpr auto description_v = meta::static_string{ "Override the " } + builder_type::template get_str<"name">() + meta::static_string{ " tool" };
			return cli::option
				.name<cli()>()
				.template description<description_v>()
				.template parse<cli()>();
		}
		
		using option_type = decltype(option());
	};
	static constexpr auto tool = tool_impl<>{};
}

#endif // MGMAKE_TOOL_TOOL_HXX