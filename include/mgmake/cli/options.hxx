#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

#include "../meta/static_dict.hxx"
#include "../meta/value_list.hxx"

#include <type_traits>
#include <utility>

namespace mgmake::cli {
	// Type list of all options for the build program
	template<typename storage_t = meta::static_dict<>>
	struct options_impl {
		// The storage type for the option values
		using storage_type = storage_t;

		template<meta::static_string key_v>
		static consteval decltype(auto) has() {
			return storage_type::template has<key_v>();
		}

		template<meta::static_string key_v>
		constexpr decltype(auto) get() {
			return m_storage.template get<key_v>();
		}
		template<meta::static_string key_v>
		constexpr decltype(auto) get() const {
			return m_storage.template get<key_v>();
		}
		template<meta::static_string key_v>
		constexpr void set(auto&& value) {
			return m_storage.template set<key_v>(std::forward<decltype(value)>(value));
		}

		// Overloads for option types
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		static inline consteval decltype(auto) has() {
			return has<opt_v.storage_key()>();
		}
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		constexpr decltype(auto) get() {
			static_assert(has<opt_v>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template get<opt_v.storage_key()>();
		}
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		constexpr decltype(auto) get() const {
			static_assert(has<opt_v>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template get<opt_v.storage_key()>();
		}
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		constexpr void set(auto&& value) {
			static_assert(has<opt_v>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template set<opt_v.storage_key()>(std::forward<decltype(value)>(value));
		}

		/*
		constexpr options_impl() {
			list_type::for_each([&]<auto opt_v> constexpr {
				if constexpr (opt_v.has_storage) {
					if constexpr (not std::is_same_v<decltype(opt_v.default_value()), std::nullopt_t>) {
						constexpr auto key = opt_v.storage_key();
						set<key>(opt_v.default_value());
					}
				}
			});
		}
		*/
		
	private:
		storage_type m_storage{};
	};
}

#endif // MGMAKE_CLI_OPTIONS_HXX
