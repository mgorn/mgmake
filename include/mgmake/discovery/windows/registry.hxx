#pragma once

#ifndef MGMAKE_DISCOVERY_WINDOWS_REGISTRY_HXX
#define MGMAKE_DISCOVERY_WINDOWS_REGISTRY_HXX

#include "../providers.hxx"

#include <optional>
#include <string>
#include <string_view>

#if defined(_WIN32)
	#include "../../sys/platform.hxx"
	#pragma comment(lib, "Advapi32.lib")
#endif // defined(_WIN32)

namespace mgmake::discovery::windows {
#if defined(_WIN32)
	[[nodiscard]] inline std::optional<std::string> read_registry_string(
		HKEY hive,
		std::string_view key,
		std::string_view value_name,
		REGSAM view = KEY_WOW64_64KEY
	) {
		HKEY handle{};
		const std::string key_text{key};

		if (RegOpenKeyExA(hive, key_text.c_str(), 0, KEY_READ | view, &handle) != ERROR_SUCCESS) {
			return std::nullopt;
		}

		DWORD type = 0;
		DWORD size = 0;
		const std::string value_text{value_name};
		const char* value_ptr = value_text.empty() ? nullptr : value_text.c_str();

		if (RegQueryValueExA(handle, value_ptr, nullptr, &type, nullptr, &size) != ERROR_SUCCESS
			|| (type != REG_SZ && type != REG_EXPAND_SZ)) {
			RegCloseKey(handle);
			return std::nullopt;
		}

		std::string result(size, '\0');

		if (RegQueryValueExA(
				handle,
				value_ptr,
				nullptr,
				&type,
				reinterpret_cast<BYTE*>(result.data()),
				&size
			) != ERROR_SUCCESS) {
			RegCloseKey(handle);
			return std::nullopt;
		}

		RegCloseKey(handle);

		while (!result.empty() && result.back() == '\0') {
			result.pop_back();
		}

		return result;
	}
#endif // defined(_WIN32)
}

namespace mgmake::discovery {
	inline void add_windows_registry_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs;

		if (auto kits = windows::read_registry_string(
				HKEY_LOCAL_MACHINE,
				R"(SOFTWARE\Microsoft\Windows Kits\Installed Roots)",
				"KitsRoot10"
			)) {
			dirs.emplace_back(std::filesystem::path{*kits} / "bin");
		}

		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::windows_registry, 130, "Windows registry and registered install roots", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif // defined(_WIN32)
	}
}

#endif // MGMAKE_DISCOVERY_WINDOWS_REGISTRY_HXX
