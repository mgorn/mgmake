#include "../include/mgmake/mgmake.hxx"

#include <cassert>
#include <span>
#include <string>
#include <vector>

namespace {
	using namespace mgmake;

	[[nodiscard]] cli::parse_result parse_args(std::vector<std::string> args) {
		return cli::parse(std::span<const std::string>{ args.data(), args.size() });
	}

	using deduced_parser = cli::option_parser<
		cli::option_builder<>
			::field<&cli::options::m_verbose>
			::name<"verbose">
			::short_name<'v'>
			::description<"Verbose output.">,

		cli::option_builder<>
			::field<&cli::options::m_build_dir>
			::name<"build-dir">
			::value_name<"path">
			::description<"Build directory.">,

		cli::option_builder<>
			::field<&cli::options::m_targets>
			::name<"target">
			::value_name<"name">
			::description<"Target to build.">
	>;

	[[nodiscard]] cli::parse_result parse_deduced(std::vector<std::string> args) {
		return deduced_parser::parse(
			std::span<const std::string>{ args.data(), args.size() }
		);
	}
}

int main() {
	assert(cli::detail::option_choices_string<cli::backend_option>()
		== "auto, graphviz, ninja, make, direct");
	assert(cli::detail::option_usage_string<cli::backend_option>()
		== "--backend <name>");
	assert(cli::backend_option::description_view()
		== "Select a build backend to use.");
	assert(cli::build_dir_option::description_view()
		== "Set the build directory.");
	assert(cli::jobs_option::description_view()
		== "Set the maximum number of parallel jobs.");

	auto graphviz = parse_args({ "--backend", "graphviz" });
	assert(graphviz);
	assert(graphviz.m_value.m_backend == cli::backend_kind::graphviz);

	auto dot = parse_args({ "--backend", "dot" });
	assert(dot);
	assert(dot.m_value.m_backend == cli::backend_kind::graphviz);

	auto graph = parse_args({ "--backend", "graph" });
	assert(graph);
	assert(graph.m_value.m_backend == cli::backend_kind::graphviz);

	auto verbose_short = parse_deduced({ "-v" });
	assert(verbose_short);
	assert(verbose_short.m_value.m_verbose);

	auto verbose_long = parse_deduced({ "--verbose" });
	assert(verbose_long);
	assert(verbose_long.m_value.m_verbose);

	auto build_dir_separate = parse_deduced({ "--build-dir", "out" });
	assert(build_dir_separate);
	assert(build_dir_separate.m_value.m_build_dir == "out");

	auto build_dir_equals = parse_deduced({ "--build-dir=out" });
	assert(build_dir_equals);
	assert(build_dir_equals.m_value.m_build_dir == "out");

	auto targets = parse_deduced({ "--target", "app", "--target", "test" });
	assert(targets);
	assert((targets.m_value.m_targets == std::vector<std::string>{ "app", "test" }));

	return 0;
}
