#pragma once

#include "commands/CommandType.hpp"
#include <map>
#include <memory.h>
#include <string>

class ACommand;
class IServer;

typedef ACommand* (*CommandSpawner)(IServer&);

class CommandFactory
{
private:
	std::map< irc::CommandType, CommandSpawner > m_commandSpawners;
	std::map< std::string, irc::CommandType > m_stringToType;
	static CommandFactory* s_instance;

	CommandFactory();
	CommandFactory(const CommandFactory& source);

	~CommandFactory();
	CommandFactory& operator=(const CommandFactory& source);

public:
	static CommandFactory* getInstance();
	static void destroyInstance();

	void registerCommandSpawner(const std::string& commandName, irc::CommandType type,
								CommandSpawner spawner);
	ACommand* createCommand(irc::CommandType type, IServer& server) const;
	bool hasCommand(irc::CommandType type) const;
	irc::CommandType stringToCommandType(const std::string& commandName) const;
};
