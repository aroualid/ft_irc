#include "commands/CommandFactory.hpp"
#include "commands/NickCommand.hpp"
#include "commands/PassCommand.hpp"
#include "commands/UserCommand.hpp"
#include <map>

static CommandFactory* s_instance = NULL;

CommandFactory::CommandFactory(Server& server) : m_commands(), m_server(server)
{
	registerAllCommands();
}

void CommandFactory::registerAllCommands()
{
	m_commands["PASS"] = new PassCommand(m_server);
	m_commands["NICK"] = new NickCommand(m_server);
	m_commands["USER"] = new UserCommand(m_server);
}

CommandFactory* CommandFactory::getInstance(Server* server)
{
	if (s_instance == NULL)
	{
		if (server == NULL)
			return NULL;

		s_instance = new CommandFactory(*server);
	}

	return s_instance;
}

void CommandFactory::destroyInstance()
{
	if (s_instance)
	{
		for (std::map< std::string, ACommand* >::iterator it = s_instance->m_commands.begin();
			 it != s_instance->m_commands.end(); ++it)
		{
			delete it->second;
		}
		s_instance->m_commands.clear();

		delete s_instance;
		s_instance = NULL;
	}
}

ACommand* CommandFactory::getCommand(const std::string& name)
{
	std::map< std::string, ACommand* >::iterator it = m_commands.find(name);

	if (it != m_commands.end())
		return it->second;

	return NULL;
}

bool CommandFactory::hasCommand(const std::string& name) const
{
	return m_commands.find(name) != m_commands.end();
}
