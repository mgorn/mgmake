#include <cstdlib>
#include <format>
#include <iostream>

int main() {
	auto compiler = "clang++";
	auto output = "builder";
	auto sources = "build.cxx";
	auto standard = "c++2c";

	const auto cmd = std::format("{} -std={} -o {} {}", compiler, standard, output, sources);
	std::cout << "Invoking build command: '" << cmd << "'" << std::endl;
	const auto result = system(cmd.c_str());
	if (result != 0) {
		std::cout << "Build failed: " << result << std::endl;
	} else {
		std::cout << "Built successfully" << std::endl;
	}
	return result;
}