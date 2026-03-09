#pragma once

#ifndef MGMAKE_HXX
#error "Please include mgmake before including extensions!"
#else

namespace mgmake {
	template<
		detail::StaticString path_v = ""
	>
	struct CMakeTargetImpl {
		template<detail::StaticString new_path_v>
		using path = TargetImpl<new_path_v>;\

		static auto configure(const auto& toolchain, const auto& project) {
			return std::format("cmake -S {} -B .mgmake/cmake-build", path_v.str());
		}
		[[nodiscard]] static auto command(const auto& toolchain, const auto& project) {
			return "cmake --build .mgmake/cmake-build";
		}
		static auto build(const auto& toolchain, const auto& project) {
			// Configure the CMake project
			auto conf = configure(toolchain, project);
			std::print("Invoking cmake config: {}\n", conf);

			// Build the cmake project
			auto cmd = command(toolchain, project);
			std::print("Invoking build command: {}\n", cmd);
			return system(cmd.c_str());
		}
	};
	using CMakeTarget = CMakeTargetImpl<>;
}

#endif
