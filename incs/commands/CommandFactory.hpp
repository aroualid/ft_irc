#pragma once

#include <map>
#include <string>

class ACommand;
class IServer;

typedef ACommand* (*CommandSpawner)(IServer&);

class CommandFactory
{
private:
	std::map< std::string, CommandSpawner > m_commandspawners;
	static CommandFactory* s_instance;

	CommandFactory();
	CommandFactory(const CommandFactory& source);

	~CommandFactory();
	CommandFactory& operator=(const CommandFactory& source);

public:
	static CommandFactory* getInstance();
	static void destroyInstance();

	void registerCommand(const std::string& name, ACommand* command);
	void registerCommandSpawner(const std::string& name, CommandSpawner spawner);
	ACommand* getCommand(const std::string& name) const;
	bool hasCommand(const std::string& name) const;
};
