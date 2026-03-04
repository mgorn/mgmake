#include <cstdlib>
#include <iostream>

int main() {
	auto result = system("clang++ build.cxx -o builder");
	if (result != 0) {
		std::cout << "Build failed: " << result << std::endl;
	} else {
		std::cout << "Built successfully" << std::endl;
	}
	return result;
}