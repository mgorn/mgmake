#pragma once

#ifndef MGMAKE_CLI_ACTION_HXX
#define MGMAKE_CLI_ACTION_HXX

#include "option.hxx"

#include <cstdint>

/*
 * Why are actions seperate from normal options? (Why can't they just be callback options?)
 *
 * Option callbacks are invoked during parsing and are functions to initialize the `cli::options` structure.
 *
 * The flow:
 * main -> parse -> cli::options -> actions
 *			|-> match
 *			|-> invoke callbacks
 *
 * but options have a `action` setting? What's with that?
 * You still need to provide the actions as options to the CLI parser.
 * They simply assign the action value in the `cli::options` structure.
 * Later on, that value is consumed to invoke the respective action handler.
 *
 * To make an action:
 *		1) Create a CLI option for the action (see `default_actions.hxx` for examples)
 *		2) Assign a handler for the action
 * 
 * NOTE: You only provide the CLI option when making the action. Do not make a second similar one to the parser manually.
 */

namespace mgmake::cli {
	template<typename storage_t = meta::type_map<>>
	struct action_impl {
		// Get the CLI option for the action
	    using option_type = typename storage_t::template at<meta::type_value<meta::static_string{ "option" }>, false>;
		static_assert(option_type::action_value, "The CLI option for actions need to have `::action<true>`");

		// Get the handler for the action
	    MGMAKE_META_TYPE_CONSUMER_FIELD(handler, nullptr);
		static inline constexpr bool valid_handler = not std::is_same_v<std::decay_t<decltype(handler_value)>, std::nullptr_t>;

		static inline constexpr auto name() {
			return option_type::name_value;
		}
		static inline constexpr auto hash() {
			return name.hash();
		}
		static inline constexpr bool match(std::string_view arg) {
			return name() == arg;
		}
		static inline constexpr auto invoke(auto& opts) {
			static_assert(valid_handler, "Invoking action with an invalid handler");
			return handler_value(opts);
		}
	};

	template<typename builder_t = meta::type_builder<>>
	struct action_builder {
        using builder_type = builder_t;

		// The CLI option for the action
		template<typename option_t = option>
        using option = action_builder<typename builder_type::template set<"option", option_t>>;
		// The function that handles the action
        MGMAKE_META_TYPE_BUILDER_FIELD(action_builder, handler, auto);

		using build = typename builder_type::template build<action_impl>;
	};
	using action = action_builder<>;
}

#endif // MGMAKE_CLI_ACTION_HXX