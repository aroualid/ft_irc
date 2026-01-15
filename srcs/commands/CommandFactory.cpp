#include "commands/CommandFactory.hpp"
#include "IServer.hpp"
#include "commands/ACommand.hpp"
#include "commands/CommandType.hpp"
#include <map>
#include <new>

CommandFactory* CommandFactory::s_instance = NULL;

static std::map< std::string, irc::CommandType > create_command_map()
{
	std::map< std::string, irc::CommandType > m;
	m["PASS"] = irc::PASS;
	m["NICK"] = irc::NICK;
	m["USER"] = irc::USER;
	m["JOIN"] = irc::JOIN;
	m["PART"] = irc::PART;
	m["PRIVMSG"] = irc::PRIVMSG;
	m["NOTICE"] = irc::NOTICE;
	// and other commands go here when they're done
	return m;
}

static const std::map< std::string, irc::CommandType > g_commandMap = create_command_map();

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

CommandFactory* CommandFactory::getInstance()
{
	if (s_instance == NULL)
		s_instance = new (std::nothrow) CommandFactory();
	return (s_instance);
}

void CommandFactory::destroyInstance()
{
	if (s_instance)
	{
		delete s_instance;
		s_instance = NULL;
	}
}

void CommandFactory::registerCommandSpawner(irc::CommandType type, CommandSpawner spawner)
{
	if (spawner)
		m_commandSpawners[type] = spawner;
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

irc::CommandType CommandFactory::stringToCommandType(const std::string& commandName)
{
	std::map< std::string, irc::CommandType >::const_iterator it = g_commandMap.find(commandName);
	if (it != g_commandMap.end())
		return (it->second);
	return (irc::CMD_UNKNOWN);
}
