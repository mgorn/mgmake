#pragma once

#ifndef MGMAKE_CLI_OPTION_HXX
#define MGMAKE_CLI_OPTION_HXX

#include "../meta/type_builder.hxx"

namespace mgmake::cli {
    enum struct option_mode {
        deduce, // Auto deduce the mode based on the definition
        flag, // bool toggle on/off flag
        assign, // set text, int value, etc. assign the option value
        append, // append to the option list value. E.g. std::vector<std::string>
        callback // invoke a callback function when the option is passed
    };

    // Actual option implementation
    template<
        meta::static_string name_v = "",
        meta::static_string description_v = "",
        char short_name_v = '\0',
        option_mode mode_v = option_mode::deduce,
        auto callback_v = nullptr
    > struct option_impl {
        static constexpr auto name_value = name_v;
        static constexpr auto description_value = description_v;
        static constexpr auto short_name_value = short_name_v;
        static constexpr auto mode_value = mode_v;
        static constexpr auto callback_value = callback_v;
    };

    // Template to consume the map made with option_builder and produce an option
    template<typename storage_t = meta::type_map<>>
    struct option_consumer {
        MGMAKE_META_TYPE_CONSUMER_FIELD(name, meta::static_string{""});
        MGMAKE_META_TYPE_CONSUMER_FIELD(description, meta::static_string{""});
	    MGMAKE_META_TYPE_CONSUMER_FIELD(short_name, '\0');
	    MGMAKE_META_TYPE_CONSUMER_FIELD(mode, option_mode::deduce);
	    MGMAKE_META_TYPE_CONSUMER_FIELD(callback, nullptr);

        using build = option_impl<
            name_value,
            description_value,
            short_name_value,
            mode_value,
            callback_value
        >;
    };

    // Build a compile-time map for the option settings
    template<typename builder_t = meta::type_builder<>>
    struct option_builder {
        using builder_type = builder_t;

        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, name, meta::static_string);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, description, meta::static_string);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, short_name, char);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, mode, option_mode);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, callback, auto);

        using build = typename builder_t::template build<option_consumer>;
    };
    // default builder alias
    using option = option_builder<>;
}

#endif // MGMAKE_CLI_OPTION_HXX