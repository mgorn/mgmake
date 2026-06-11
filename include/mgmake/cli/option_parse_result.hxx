#pragma once

#ifndef MGMAKE_CLI_OPTION_PARSE_RESULT_HXX
#define MGMAKE_CLI_OPTION_PARSE_RESULT_HXX

#include <string>
#include <utility>

namespace mgmake::cli {
	struct option_parse_result {
		bool m_matched = false;
		bool m_ok = true;
		std::string m_error{};

		[[nodiscard]] static option_parse_result no_match() {
			return {};
		}

		[[nodiscard]] static option_parse_result success() {
			option_parse_result result;
			result.m_matched = true;
			result.m_ok = true;
			return result;
		}

		[[nodiscard]] static option_parse_result failure(std::string error) {
			option_parse_result result;
			result.m_matched = true;
			result.m_ok = false;
			result.m_error = std::move(error);
			return result;
		}
	};
}

#endif
