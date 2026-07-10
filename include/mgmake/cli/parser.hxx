#pragma once

#ifndef MGMAKE_CLI_PARSER_HXX
#define MGMAKE_CLI_PARSER_HXX

#include "../meta/type_list.hxx"
#include "../sys/shell.hxx"

#include <expected>
#include <optional>
#include <string>

namespace mgmake::cli {
    template<typename list_t = meta::type_list<>>
    struct parser {
        std::expected<void, std::string> parse(const sys::shell& cmd) const {
            return std::unexpected("Parser not yet implemented");
        }
    };
}

#endif // MGMAKE_CLI_PARSER_HXX