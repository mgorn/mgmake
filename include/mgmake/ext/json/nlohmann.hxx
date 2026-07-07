#pragma once

#ifndef MGMK_EXT_JSON_NLOHMANN_HXX
#define MGMK_EXT_JSON_NLOHMANN_HXX

#include "common.hxx"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// nlohmann_json_backend_t adapts nlohmann::json to the backend-independent json_value API.

namespace mgmake::ext {
	struct nlohmann_json_backend_t {
		using native_type = nlohmann::json;
	};

	template <>
	struct json_value<nlohmann_json_backend_t> {
		using backend_type = nlohmann_json_backend_t;
		using native_type = nlohmann_json_backend_t::native_type;

		native_type m_value{};

		json_value() = default;

		explicit json_value(native_type value)
			: m_value(std::move(value)) {}

		[[nodiscard]] static std::optional<json_value> parse(std::string_view text) {
			auto parsed = native_type::parse(
				text.begin(),
				text.end(),
				nullptr,
				false
			);

			if (parsed.is_discarded()) {
				return std::nullopt;
			}

			return json_value(parsed);
		}

		operator bool() const {
			return not is_null();
		}

		[[nodiscard]] bool is_null() const noexcept {
			return m_value.is_null();
		}

		[[nodiscard]] bool is_object() const noexcept {
			return m_value.is_object();
		}

		[[nodiscard]] bool is_array() const noexcept {
			return m_value.is_array();
		}

		[[nodiscard]] bool is_string() const noexcept {
			return m_value.is_string();
		}

		[[nodiscard]] bool is_boolean() const noexcept {
			return m_value.is_boolean();
		}

		[[nodiscard]] bool is_number() const noexcept {
			return m_value.is_number();
		}

		[[nodiscard]] bool is_integer() const noexcept {
			return m_value.is_number_integer();
		}

		[[nodiscard]] bool has(const std::string& key) const {
			mgmkassert(m_value.is_object(), "json_value::has can only be used on objects");
			return m_value.contains(key);
		}

		[[nodiscard]] json_value get(const std::string& key) const {
			mgmkassert(m_value.is_object(), "json_value::get can only be used on objects");

			mgmkassert(has(key), std::format("json_value::get member '{}' doesn't exist", key));
			const auto found = m_value.find(key);
			mgmkassert(found != m_value.end(), std::format("json_value::get failed to get member '{}'", key));

			return json_value(*found);
		}

		[[nodiscard]] std::optional<json_value> operator[](const std::string& key) const {
			return get(key);
		}

		[[nodiscard]] std::vector<json_value> array(const std::string& key) const {
			std::vector<json_value> result;
			const auto value = get(key);
			mgmkassert(value.is_array(), std::format("Requested key: '{}' is not an array", key));
			return value.items();
		}

		[[nodiscard]] std::vector<json_value> items() const {
			std::vector<json_value> result;

			mgmkassert(is_array(), "json_value::items expects m_value to be an array");

			// store each array item in the result
			for (const auto& item : m_value) {
				result.emplace_back(json_value(item));
			}

			return result;
		}

		template <typename value_t>
		[[nodiscard]] std::optional<value_t> as() const {
			try {
				return m_value.template get<value_t>();
			} catch (...) {
				return std::nullopt;
			}
		}

		[[nodiscard]] std::optional<std::string> as_string() const {
			mgmkassert(is_string(), "json_value::as_string failed, value is not a string");
			return as<std::string>();
		}

		[[nodiscard]] std::optional<bool> as_bool() const {
			mgmkassert(is_boolean(), "json_value::as_bool failed, value is not a bool");
			return as<bool>();
		}

		[[nodiscard]] std::optional<int> as_int() const {
			mgmkassert(is_integer(), "json_value::as_int failed, value is not an int");
			return as<int>();
		}

		[[nodiscard]] std::optional<std::int64_t> as_i64() const {
			mgmkassert(is_integer(), "json_value::as_i64 failed, value is not an int");
			return as<std::int64_t>();
		}

		[[nodiscard]] std::optional<std::uint64_t> as_u64() const {
			mgmkassert(m_value.is_number_unsigned(), "json_value::as_u64 failed, value is not a std::uint64_t");
			return as<std::uint64_t>();
		}

		[[nodiscard]] std::optional<double> as_double() const {
			mgmkassert(is_number(), "json_value::as_double failed, value is not a double/number");
			return as<double>();
		}
	};
}

#endif // MGMK_EXT_JSON_NLOHMANN_HXX
