#pragma once

#ifndef MGMAKE_FS_FILE_HXX
#define MGMAKE_FS_FILE_HXX

#include <filesystem>
#include <fstream>

namespace mgmake::fs {
    using path = std::filesystem::path;

    // Convenience wrapper around standard C++ filesystem & io types
    template<typename stream_t = std::ifstream>
    struct stream {
        using stream_type = stream_t;
        static inline constexpr bool is_input = std::is_same_v<stream_type, std::ifstream>;
        static inline constexpr bool is_output = std::is_same_v<stream_type, std::ofstream>;

        stream_type m_stream{}:

        inline auto is_open() const {
            return m_stream.is_open();
        }
        inline auto is_good() const {
            return m_stream.good();
        }

        auto read(auto&&... args) const {
            return m_stream.read(std::forward<decltype(args)>(args)...);
        }

        std::string read_all() const {
            return { std::istreambuf_iterator<char>{ m_stream }, std::istreambuf_iterator<char>{} };
        }

        template<typename value_t, auto manip_v = nullptr>
        value_t get() const {
            static_assert(is_input, "fs::stream::get can only be used with input streams");
            value_t result;
            if constexpr (manip_v) {
                m_stream >> manip_v(result);
            } else {
                m_stream >> result;
            }
            return std::move(result);
        }
    };

    // Open a file stream
    template<std::ios_base::openmode mode_v = std::ios::binary, typename std_stream_t = std::ifstream, typename stream_t = fs::stream<std_stream_t, mode>>
    static inline std::optional<stream_t> open(const path& path) {
        stream_t s{ 
            .m_stream = std_stream_t{ path, mode_v }
        };
        if (s.is_open() && s.is_good()) {
            return s;
        }
        return std::nullopt;
    }
}

#endif