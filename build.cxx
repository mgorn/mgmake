#include <mgmake.hxx>
#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <vector>

struct Toolchain {
	std::string mCompiler = "clang++";
};

template<
	mgmake::detail::StaticString... sources_v
>
struct ListImpl {
	template<mgmake::detail::StaticString new_source_v>
	[[nodiscard]] constexpr auto add() const {
		return mgmake::detail::poof<ListImpl<sources_v..., new_source_v>>();
	}

	[[nodiscard]] constexpr auto collect() const {
		if constexpr (sizeof...(sources_v) == 0) {
			return mgmake::detail::StaticString{""};
		} else if constexpr (sizeof...(sources_v) == 1) {
			return (sources_v + ...);
		} else {
			constexpr mgmake::detail::StaticString space{" "};
			return ((sources_v + space) + ...);
		}
	}
};
static constexpr ListImpl Sources{};
static constexpr ListImpl Includes{};

// Assertions to make sure sources are collected properly
static_assert(Sources.add<"build.cxx">().collect().str() == "build.cxx");
static_assert(Sources.add<"build.cxx">().add<"another.cxx">().collect().str() == "build.cxx another.cxx ");

template<
	mgmake::detail::StaticString name_v = "",
	auto sources_v = Sources,
	auto includes_v = Includes
>
struct TargetImpl {
	template<mgmake::detail::StaticString new_name_v>
	using name = TargetImpl<new_name_v, sources_v, includes_v>;
	template<auto new_sources_v = Sources>
	using sources = TargetImpl<name_v, new_sources_v, includes_v>;
	template<auto new_includes_v = Includes>
	using includes = TargetImpl<name_v, sources_v, new_includes_v>;

	[[nodiscard]] static auto command(const Toolchain& toolchain, const auto& project) {
		return std::format("{} -std={} -o {} {} -I {}", toolchain.mCompiler, project.standard_value.str(), name_v.str(), sources_v.collect().str(), includes_v.collect().str());
	}
	static auto build(const Toolchain& toolchain, const auto& project) {
		auto cmd = command(toolchain, project);
		std::print("Invoking build command: {}\n", cmd);
		return system(cmd.c_str());
	}
};
using Target = TargetImpl<>;

template<
	mgmake::detail::StaticString name_v = "",
	mgmake::detail::StaticString standard_v = "c++2c",
	typename... target_ts
>
struct ProjectImpl {
	template<mgmake::detail::StaticString new_name_v>
	using name = ProjectImpl<new_name_v, standard_v, target_ts...>;

	template<mgmake::detail::StaticString new_standard_v>
	using standard = ProjectImpl<name_v, new_standard_v, target_ts...>;

	template<typename new_target_t>
	using add_target = ProjectImpl<name_v, standard_v, target_ts..., new_target_t>;

	static constexpr auto name_value = name_v;
	static constexpr auto standard_value = standard_v;

	[[nodiscard]] auto build(const Toolchain& toolchain) const {
		std::print("Building project '{}'\n", name_value.str());
		return (target_ts::build(toolchain, *this) | ...);
	}
};
using Project = ProjectImpl<>;

using Builder = Target::name<"builder">
	::sources<Sources.add<"build.cxx">()>
	::includes<Includes.add<"include">()>;

using MgMake = Project
	::name<"mgmake">
	::standard<"c++2c">
	::add_target<Builder>;

int main() {
	const Toolchain toolchain;
	const MgMake project;
	return project.build(toolchain);

	//std::cout << Sources.add<"build.cxx">().collect().str() << std::endl;
	/*Project project{ "mgmake", "c++2c", { { "builder", {
			{ "build.cxx" }
	} } } } ;*/

	//auto result = 0;
	/*
	for (const auto& target : project.mTargets) {
		const auto cmd = target.command(toolchain, project);
		std::cout << "Invoking build command: '" << cmd << "'" << std::endl;
		result |= system(cmd.c_str());
		if (result != 0) {
			std::cout << "Build failed: " << result << std::endl;
		} else {
			std::cout << "Built successfully" << std::endl;
		}
	}
	*/
	//return result;
}