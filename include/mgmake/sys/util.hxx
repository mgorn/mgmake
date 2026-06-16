#pragma once

#ifndef MGMAKE_SYS_UTIL_HXX
#define MGMAKE_SYS_UTIL_HXX

#include "platform.hxx"

#include <string>
#include <string_view>

namespace mgmake::sys {
#ifdef MGMK_PLATFORM_WINDOWS
	inline constexpr std::string shell_escape(std::string_view arg) {
		if (arg.empty()) {
			return "\"\"";
		}

		bool needs_quotes = false;

		for (const char ch : arg) {
			if (
				ch == ' '
				|| ch == '\t'
				|| ch == '"'
				|| ch == '&'
				|| ch == '|'
				|| ch == '<'
				|| ch == '>'
				|| ch == '^'
				|| ch == '%'
				|| ch == '!'
				|| ch == '('
				|| ch == ')'
			) {
				needs_quotes = true;
				break;
			}
		}

		if (!needs_quotes) {
			return std::string(arg);
		}

		std::string result;
		result += '"';

		std::size_t backslashes = 0;

		for (const char ch : arg) {
			if (ch == '\\') {
				++backslashes;
				continue;
			}

			if (ch == '"') {
				result.append(backslashes * 2 + 1, '\\');
				result += '"';
				backslashes = 0;
				continue;
			}

			if (backslashes != 0) {
				result.append(backslashes, '\\');
				backslashes = 0;
			}

			result += ch;
		}

		if (backslashes != 0) {
			result.append(backslashes * 2, '\\');
		}

		result += '"';
		return result;
	}
#else
	inline constexpr std::string shell_escape(std::string_view arg) {
		if (arg.empty()) {
			return "''";
		}

		bool needs_quotes = false;

		for (const char ch : arg) {
			if (ch == ' ' || ch == '\t' || ch == '\'' || ch == '"' || ch == '$' || ch == '\\' || ch == '&' || ch == ';' || ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '|') {
				needs_quotes = true;
				break;
			}
		}

		if (!needs_quotes) {
			return std::string(arg);
		}

		std::string result;
		result += '\'';

		for (const char ch : arg) {
			if (ch == '\'') {
				result += "'\\''";
			} else {
				result += ch;
			}
		}

		result += '\'';
		return result;
	}
#endif
}

#endif
