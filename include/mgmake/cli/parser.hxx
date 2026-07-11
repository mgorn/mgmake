#pragma once

#ifndef MGMAKE_CLI_PARSER_HXX
#define MGMAKE_CLI_PARSER_HXX

#include "options.hxx"

#include "../meta/type_list.hxx"
#include "../sys/shell.hxx"

#include <bitset>
#include <expected>
#include <optional>
#include <string>

namespace mgmake::cli {
    template<typename list_t = meta::type_list<>>
    struct parser {
        static inline constexpr std::expected<options, std::string> parse(const sys::shell& cmd) {
			auto args = cmd.user_args();
			for (std::string_view arg : args) {
				auto matches = match(arg);
				if (matches.any()) {
                    // TODO: Take the matched options, handle them
					return std::unexpected("Matched!");
				}
			}
            return std::unexpected("Parser not yet implemented");
        }

		using matches_type = std::bitset<list_t::size()>;
		static inline constexpr matches_type match(std::string_view arg) {
			return []<std::size_t... Is>(std::index_sequence<Is...>, std::string_view arg) {
				matches_type matches{};
				(matches.set(Is, list_t::template type_at<Is>::match(arg)), ...);
				return matches;
			}(std::make_index_sequence<list_t::size()>{}, arg);
		}
    };
}

#endif // MGMAKE_CLI_PARSER_HXX