#pragma once

#ifndef MGMAKE_ENTRY_EXIT_CODE_HXX
#define MGMAKE_ENTRY_EXIT_CODE_HXX

// Entry exit codes keep usage errors separate from build/action failures.

namespace mgmake::detail {
	inline constexpr int entry_exit_success = 0;
	inline constexpr int entry_exit_action_failure = 1;
	inline constexpr int entry_exit_usage_error = 2;
}

#endif // MGMAKE_ENTRY_EXIT_CODE_HXX
