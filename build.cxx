#include <mgmake.hxx>
#include <format>
#include <iostream>
#include <string>
#include <vector>

struct Toolchain {
	std::string mCompiler = "clang++";
};

struct Sources {
	std::vector<std::string> mSources{};

	[[nodiscard]] std::string collect() const {
		std::string result;
		for (const auto& source : mSources) {
			result += source + " ";
		}
		return result;
	}
};

struct Target;

template<
	mgmake::detail::StaticString name_v = "",
	mgmake::detail::StaticString standard_v = "c++2c",
	auto... targets_v
>
struct ProjectImpl {
	template<mgmake::detail::StaticString new_name_v>
	using name = ProjectImpl<new_name_v, standard_v, targets_v...>;

	template<mgmake::detail::StaticString new_standard_v>
	using standard = ProjectImpl<name_v, new_standard_v, targets_v...>;

	static constexpr auto name_value = name_v;
	static constexpr auto standard_value = standard_v;
};
using Project = ProjectImpl<>;

struct Target {
	std::string mName;
	Sources mSources{};
	std::vector<Target*> mDependencies{};
	std::vector<std::string> mIncludePaths{};


	[[nodiscard]] auto command(const Toolchain& toolchain, const auto& project) const {
		return std::format("{} -std={} -o {} {}", toolchain.mCompiler, project.standard(), mName, mSources.collect());
	}
};

using MgMake = Project::name<"mgmake">::standard<"c++2c">;

int main() {
	Toolchain toolchain;
	/*Project project{ "mgmake", "c++2c", { { "builder", {
			{ "build.cxx" }
	} } } } ;*/

	auto result = 0;
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
	return result;
}