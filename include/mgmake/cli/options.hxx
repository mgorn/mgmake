#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

#include "action.hxx"

namespace mgmake::cli {
	// Store parsed CLI options
	struct options {
		// Build action by default
		action::kind m_action = action::kind::build;
	};
}

#endif // MGMAKE_CLI_OPTIONS_HXX