#pragma once

#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"

namespace detail
{
// helper to check if a class is derived from a base (which is what you should be doing)
// ((shame on you otherwise))
template < typename BaseCommand, typename ConcreteCommand > struct IsDerivedFrom
{
	static void constraints(ConcreteCommand* p)
	{
		BaseCommand* bp = p;
		(void)bp;
	}
};
} // namespace detail

// helper macro that does the dirty work so commands get to register themselves like grown ups :D
// basically this allows them to grab an instance of CommandFactory and register their name + create
// function. since the registrar is instantiated statically, it lives on the bss and therefore the
// registration happens before main even starts. isn't that awesome?
//
// CommandClass must obviously inherit from ACommand, otherwise it would be pretty pointless imo
//
// tldr; command calls CommandFactory and goes:
// "heeeeey~ im <insert command name> and that's how you instantiate me *wink wink ;)*"

#define REGISTER_COMMAND(CommandClass, CommandType, CommandName)                                   \
	namespace                                                                                      \
	{                                                                                              \
	class CommandRegistrar_##CommandClass                                                          \
	{                                                                                              \
	public:                                                                                        \
		CommandRegistrar_##CommandClass()                                                          \
		{                                                                                          \
			(void)sizeof(detail::IsDerivedFrom< ACommand, CommandClass >);                         \
			CommandFactory::getInstance().registerCommandSpawner(CommandName, CommandType,        \
																  &CommandClass::create);          \
		}                                                                                          \
	};                                                                                             \
	static CommandRegistrar_##CommandClass g_registrar_##CommandClass;                             \
	}
