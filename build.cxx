#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <vector>

struct Toolchain {
	std::string mCompiler = "clang++";
};

struct Sources {
	std::vector<std::string> mSources{};

	operator std::string() const {
		return str();
	}
	[[nodiscard]] std::string str() const {
		std::string result;
		for (const auto& source : mSources) {
			result += source + " ";
		}
		return result;
	}
};

struct Target {
	std::string mName;
	Sources mSources{};
};

int main() {
	Toolchain toolchain;
	Target target{ "builder", {
		{ "build.cxx" }
	} };
	auto standard = "c++2c";

	const auto cmd = std::format("{} -std={} -o {} {}", toolchain.mCompiler, standard, target.mName, target.mSources.str());
	std::cout << "Invoking build command: '" << cmd << "'" << std::endl;
	const auto result = system(cmd.c_str());
	if (result != 0) {
		std::cout << "Build failed: " << result << std::endl;
	} else {
		std::cout << "Built successfully" << std::endl;
	}
	return result;
}