#pragma once

#ifndef MGMK_EXT_JSON_COMMON_HXX
#define MGMK_EXT_JSON_COMMON_HXX

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Common JSON wrappers define the backend-independent shape used by extension parsers.

namespace mgmake::ext {
	struct no_json_backend_t {
		struct native_type {};
	};

	template <typename backend_t>
	struct json_value {
		using backend_type = backend_t;
		using native_type = typename backend_t::native_type;

		native_type m_value{};

		[[nodiscard]] static constexpr std::optional<json_value> parse(std::string_view) {
			return std::nullopt;
		}

		[[nodiscard]] constexpr bool is_null() const noexcept {
			return false;
		}

		[[nodiscard]] constexpr bool is_object() const noexcept {
			return false;
		}

		[[nodiscard]] constexpr bool is_array() const noexcept {
			return false;
		}

		[[nodiscard]] constexpr bool is_string() const noexcept {
			return false;
		}

		[[nodiscard]] constexpr bool is_boolean() const noexcept {
			return false;
		}

		[[nodiscard]] constexpr bool is_number() const noexcept {
			return false;
		}

		[[nodiscard]] constexpr bool is_integer() const noexcept {
			return false;
		}

		[[nodiscard]] constexpr bool has(std::string_view) const {
			return false;
		}

		[[nodiscard]] constexpr std::optional<json_value> get(std::string_view) const {
			return std::nullopt;
		}

		[[nodiscard]] constexpr std::optional<json_value> operator[](std::string_view key) const {
			return get(key);
		}

		[[nodiscard]] constexpr std::vector<json_value> array(std::string_view) const {
			return {};
		}

		[[nodiscard]] constexpr std::vector<json_value> items() const {
			return {};
		}

		template <typename value_t>
		[[nodiscard]] constexpr std::optional<value_t> as() const {
			return std::nullopt;
		}

		[[nodiscard]] constexpr std::optional<std::string> as_string() const {
			return as<std::string>();
		}

		[[nodiscard]] constexpr std::optional<bool> as_bool() const {
			return as<bool>();
		}

		[[nodiscard]] constexpr std::optional<int> as_int() const {
			return as<int>();
		}

		[[nodiscard]] constexpr std::optional<std::int64_t> as_i64() const {
			return as<std::int64_t>();
		}

		[[nodiscard]] constexpr std::optional<std::uint64_t> as_u64() const {
			return as<std::uint64_t>();
		}

		[[nodiscard]] constexpr std::optional<double> as_double() const {
			return as<double>();
		}
	};
}

#endif // MGMK_EXT_JSON_COMMON_HXX
