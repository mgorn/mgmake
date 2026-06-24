#pragma once

#ifndef MGMAKE_CLI_PARSE_RESULT_HXX
#define MGMAKE_CLI_PARSE_RESULT_HXX

#include "options.hxx"

#include <string>
#include <utility>

// The parser returns either fully parsed options or a user-facing error string.

namespace mgmake::cli {
	struct parse_result {
		bool m_ok = false;
		options m_value{};
		std::string m_error{};

		[[nodiscard]] operator bool() const {
			return m_ok;
		}

		[[nodiscard]] operator options() const {
			return m_value;
		}

		static parse_result success(options opts) {
			parse_result result;
			result.m_ok = true;
			result.m_value = std::move(opts);
			return result;
		}

		static parse_result failure(std::string message) {
			parse_result result;
			result.m_ok = false;
			result.m_error = std::move(message);
			return result;
		}
	};
}

#endif // MGMAKE_CLI_PARSE_RESULT_HXX
