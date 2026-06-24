#pragma once

#ifndef MGMAKE_DISCOVERY_WINDOWS_VISUAL_STUDIO_HXX
#define MGMAKE_DISCOVERY_WINDOWS_VISUAL_STUDIO_HXX

#include "../providers.hxx"
#include "../validate.hxx"

#include <ranges>
#include <sstream>
#include <utility>

namespace mgmake::discovery::windows {
	struct visual_studio_instance {
		std::filesystem::path m_root{};
		std::string m_version{};
		std::string m_display_name{};
		bool m_has_cpp_tools = false;
	};

	[[nodiscard]] inline std::optional<std::filesystem::path> find_vswhere() {
		if (auto override = getenv_path("MGMK_VSWHERE")) {
			if (is_launchable_file(*override)) return std::filesystem::absolute(*override);
		}

		for (const auto& dir : path_entries()) {
			if (auto found = find_in_directory(dir, "vswhere")) return found;
		}

		std::vector<std::filesystem::path> candidates;

		if (auto pf86 = getenv_path("ProgramFiles(x86)")) {
			candidates.emplace_back(*pf86 / "Microsoft Visual Studio" / "Installer" / "vswhere.exe");
		}

		if (auto pf = getenv_path("ProgramFiles")) {
			candidates.emplace_back(*pf / "Microsoft Visual Studio" / "Installer" / "vswhere.exe");
		}

		for (const auto& candidate : candidates) {
			if (is_launchable_file(candidate)) return std::filesystem::absolute(candidate);
		}

		return std::nullopt;
	}

	[[nodiscard]] inline std::vector<visual_studio_instance> common_visual_studio_instances() {
		std::vector<visual_studio_instance> result;

#if defined(_WIN32)
		std::vector<std::filesystem::path> roots;
		if (auto pf = getenv_path("ProgramFiles")) roots.emplace_back(*pf / "Microsoft Visual Studio" / "2022");
		if (auto pf86 = getenv_path("ProgramFiles(x86)")) roots.emplace_back(*pf86 / "Microsoft Visual Studio" / "2019");

		for (const auto& year_root : roots) {
			for (std::string edition : {"Community", "Professional", "Enterprise", "BuildTools"}) {
				auto root = year_root / edition;
				if (std::filesystem::exists(root)) {
					result.push_back({root, {}, edition, std::filesystem::exists(root / "VC" / "Tools" / "MSVC")});
				}
			}
		}
#endif // defined(_WIN32)

		return result;
	}

	[[nodiscard]] inline std::vector<std::filesystem::path> vswhere_installation_paths() {
		std::vector<std::filesystem::path> result;

#if defined(_WIN32)
		auto vswhere = find_vswhere();

		if (!vswhere) {
			return result;
		}

		sys::command_line command;
		command.m_args.emplace_back(vswhere->string());
		command.m_args.emplace_back("-all");
		command.m_args.emplace_back("-products");
		command.m_args.emplace_back("*");
		command.m_args.emplace_back("-requires");
		command.m_args.emplace_back("Microsoft.VisualStudio.Component.VC.Tools.x86.x64");
		command.m_args.emplace_back("-property");
		command.m_args.emplace_back("installationPath");

		auto text = capture_command(std::move(command));

		if (!text) {
			return result;
		}

		std::istringstream in{*text};
		std::string line;

		while (std::getline(in, line)) {
			while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
				line.pop_back();
			}

			if (!line.empty() && std::filesystem::exists(line)) {
				result.emplace_back(line);
			}
		}
#endif // defined(_WIN32)

		return result;
	}

	[[nodiscard]] inline bool same_existing_path(
		const std::filesystem::path& left,
		const std::filesystem::path& right
	) {
		std::error_code ec;
		const bool same = std::filesystem::equivalent(left, right, ec);
		return !ec && same;
	}

	[[nodiscard]] inline std::vector<visual_studio_instance> visual_studio_instances() {
		std::vector<visual_studio_instance> result;

		for (const auto& path : vswhere_installation_paths()) {
			result.push_back({
				.m_root = path,
				.m_display_name = path.filename().string(),
				.m_has_cpp_tools = std::filesystem::exists(path / "VC" / "Tools" / "MSVC")
			});
		}

		for (auto fallback : common_visual_studio_instances()) {
			const bool already_added = std::ranges::any_of(result, [&](const auto& instance) {
				return same_existing_path(instance.m_root, fallback.m_root);
			});

			if (!already_added) {
				result.emplace_back(std::move(fallback));
			}
		}

		return result;
	}

	[[nodiscard]] inline std::vector<std::filesystem::path> visual_studio_tool_dirs(
		const visual_studio_instance& vs
	) {
		std::vector<std::filesystem::path> result;
		auto msvc_root = vs.m_root / "VC" / "Tools" / "MSVC";
		std::error_code ec;

		if (std::filesystem::exists(msvc_root, ec)) {
			for (const auto& entry : std::filesystem::directory_iterator(msvc_root, ec)) {
				if (!entry.is_directory()) continue;
				const auto root = entry.path() / "bin";
				result.emplace_back(root / "Hostx64" / "x64");
				result.emplace_back(root / "Hostx64" / "x86");
				result.emplace_back(root / "Hostx64" / "arm64");
				result.emplace_back(root / "Hostx86" / "x86");
				result.emplace_back(root / "Hostx86" / "x64");
			}
		}

		result.emplace_back(vs.m_root / "VC" / "Tools" / "Llvm" / "bin");
		result.emplace_back(vs.m_root / "VC" / "Tools" / "Llvm" / "x64" / "bin");
		return result;
	}

	[[nodiscard]] inline tool_environment visual_studio_environment(
		const visual_studio_instance& vs,
		const build::request& req
	) {
		tool_environment result;
		auto vcvarsall = vs.m_root / "VC" / "Auxiliary" / "Build" / "vcvarsall.bat";

		if (is_launchable_file(vcvarsall)) {
			result.m_setup_script = vcvarsall;

			switch (req.target().m_arch) {
				case sys::arch::x86: result.m_setup_args.emplace_back("x86"); break;
				case sys::arch::aarch64: result.m_setup_args.emplace_back("arm64"); break;
				default: result.m_setup_args.emplace_back("x64"); break;
			}
		}

		return result;
	}

	[[nodiscard]] inline std::optional<std::filesystem::path> visual_studio_root_from_tools(
		const build::request& req
	) {
		for (const auto role : {
				tool_role::c_compiler,
				tool_role::cxx_compiler,
				tool_role::librarian,
				tool_role::archiver,
				tool_role::linker,
				tool_role::shared_linker
			}) {
			const auto* tool = req.discovered_tool(role);

			if (tool != nullptr && tool->m_provider_root.has_value()) {
				return tool->m_provider_root;
			}
		}

		return std::nullopt;
	}
}

namespace mgmake::discovery {
	inline void add_vswhere_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		if (req.m_logical_name != "vswhere") {
			return;
		}

		if (auto vswhere = windows::find_vswhere()) {
			out.push_back({req.m_role, req.m_logical_name.empty() ? "vswhere" : req.m_logical_name, *vswhere, tool_provider::vswhere, "Visual Studio locator", 140});
		}
#else
		(void)ctx; (void)req; (void)out;
#endif // defined(_WIN32)
	}

	inline void add_visual_studio_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		for (const auto& vs : windows::visual_studio_instances()) {
			add_candidates_from_dirs(out, ctx.request(), req, windows::visual_studio_tool_dirs(vs), tool_provider::visual_studio, 150, "Visual Studio tool directory", true, ctx.m_mode, vs.m_root);
		}
#else
		(void)ctx; (void)req; (void)out;
#endif // defined(_WIN32)
	}

	inline void add_standalone_llvm_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("MGMK_LLVM_ROOT")) dirs.emplace_back(*root / "bin");
		dirs.emplace_back("C:/Program Files/LLVM/bin");
		dirs.emplace_back("C:/Program Files (x86)/LLVM/bin");
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::standalone_llvm, 170, "standalone LLVM installation", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif // defined(_WIN32)
	}
}

#endif // MGMAKE_DISCOVERY_WINDOWS_VISUAL_STUDIO_HXX
