#pragma once

#include "ACommand.hpp"
#include <map>
#include <string>

class CommandFactory
{
	private:
		std::map<std::string, ACommand *> m_commands;

		CommandFactory(Server *server);
		void registerAllCommands();
		
	public:
		CommandFactory *getInstance(Server *server = NULL);
		static void destroyInstance();

		ACommand *getCommand(const std::string &name);
		bool hasCommand(const std::string &name) const;
};
