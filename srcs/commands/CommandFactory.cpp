#include "commands/CommandFactory.hpp"
#include "IServer.hpp"
#include "commands/ACommand.hpp"
#include "commands/CommandType.hpp"
#include <map>
#include <new>


CommandFactory::CommandFactory()
{
}

CommandFactory::CommandFactory(const CommandFactory& source)
{
	(void)source;
}

CommandFactory::~CommandFactory()
{
	m_commandSpawners.clear();
}

CommandFactory& CommandFactory::operator=(const CommandFactory& source)
{
	(void)source;
	return (*this);
}

CommandFactory &CommandFactory::getInstance()
{
	static CommandFactory instance;
	return (instance);
}

void CommandFactory::registerCommandSpawner(const std::string& commandName, irc::CommandType type,
											CommandSpawner spawner)
{
	if (spawner)
	{
		m_commandSpawners[type] = spawner;
		m_stringToType[commandName] = type;
	}
}

ACommand* CommandFactory::createCommand(irc::CommandType type, IServer& server) const
{
	std::map< irc::CommandType, CommandSpawner >::const_iterator it = m_commandSpawners.find(type);

	if (it != m_commandSpawners.end())
		return (it->second(server));

	return (NULL);
}

bool CommandFactory::hasCommand(irc::CommandType type) const
{
	return (m_commandSpawners.count(type) > 0);
}

irc::CommandType CommandFactory::stringToCommandType(const std::string& commandName) const
{
	std::map< std::string, irc::CommandType >::const_iterator it = m_stringToType.find(commandName);
	if (it != m_stringToType.end())
		return (it->second);
	return (irc::CMD_UNKNOWN);
}
