#pragma once

#ifndef MGMAKE_CLI_OPTION_HXX
#define MGMAKE_CLI_OPTION_HXX

#include "../meta/member_access.hxx"
#include "../meta/type_builder.hxx"

namespace mgmake::cli {
    enum struct option_mode {
        deduce, // Auto deduce the mode based on the definition
        flag, // bool toggle on/off flag
        assign, // set text, int value, etc. assign the option value
        append, // append to the option list value. E.g. std::vector<std::string>
        callback // invoke a callback function when the option is passed
    };

    // Actual option impl, consume the configuration in the type map
    template<typename storage_t = meta::type_map<>>
    struct option_impl {
        MGMAKE_META_TYPE_CONSUMER_FIELD(name, meta::static_string{""});
        MGMAKE_META_TYPE_CONSUMER_FIELD(description, meta::static_string{""});
	    MGMAKE_META_TYPE_CONSUMER_FIELD(short_name, '\0');
	    MGMAKE_META_TYPE_CONSUMER_FIELD(mode, option_mode::deduce);
	    MGMAKE_META_TYPE_CONSUMER_FIELD(callback, nullptr);
	    //MGMAKE_META_TYPE_CONSUMER_FIELD(assign, meta::member_access<>);

		static inline constexpr bool match(std::string_view arg) {
			if (arg.empty()) {
				return false;
			}

			if (arg.starts_with("--")) {
				arg.remove_prefix(2);
			} else if (arg.starts_with("-")) {
				arg.remove_prefix(1);
			}

			return arg == name_value or (arg.size() == 1 and arg.front() == short_name_value);
		}
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
		// TODO: Not use the macro so the value to assign can be passed as well?
		// Or wait --assigned=value assigns parsed value to the member
		// so A seperate one that assigns a fixed value as well? Or just use callback to override?
        //MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, assign, meta::member_access<>);

        using build = typename builder_t::template build<option_impl>;
    };
    // default builder alias
    using option = option_builder<>;
}

#endif // MGMAKE_CLI_OPTION_HXX