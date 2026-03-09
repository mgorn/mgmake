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

struct Project {
	std::string mName;
	std::string mStandard = "c++2c";
	std::vector<Target> mTargets{};
	std::vector<std::string> mIncludePaths{};
};

struct Target {
	std::string mName;
	Sources mSources{};
	std::vector<Target*> mDependencies{};

	
	[[nodiscard]] auto command(const Toolchain &toolchain, const Project &project) const {
		return std::format("{} -std={} -o {} {}", toolchain.mCompiler, project.mStandard, mName, mSources.collect());
	}
};

int main() {
	Toolchain toolchain;
	Project project{ "mgmake", "c++2c", { { "builder", {
			{ "build.cxx" }
	} } } } ;

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