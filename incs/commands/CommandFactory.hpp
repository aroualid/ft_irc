#pragma once

#include "ACommand.hpp"
#include "IServer.hpp"
#include <map>
#include <string>

class CommandFactory
{
private:
	std::map< std::string, ACommand* > m_commands;
	IServer& m_server;

	CommandFactory(IServer& server);
	void registerAllCommands();

public:
	static CommandFactory* getInstance(IServer* server = NULL);
	static void destroyInstance();

	ACommand* getCommand(const std::string& name);
	bool hasCommand(const std::string& name) const;
};
