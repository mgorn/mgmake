#pragma once

#ifndef MGMAKE_SYS_EXIT_CODE_HXX
#define MGMAKE_SYS_EXIT_CODE_HXX

namespace mgmake::sys {
    enum struct exit_code : int {
        success,
        task_failure,
        usage_error
    };
}

#endif