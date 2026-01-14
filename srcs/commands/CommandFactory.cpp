#include "commands/CommandFactory.hpp"
#include "commands/ACommand.hpp"
#include <map>
#include <new>

CommandFactory* CommandFactory::s_instance = NULL;

CommandFactory::CommandFactory()
{
}

CommandFactory::CommandFactory(const CommandFactory& source)
{
	(void)source;
}

CommandFactory::~CommandFactory()
{
	for (std::map< std::string, ACommand* >::iterator it = s_instance->m_commands.begin();
		 it != s_instance->m_commands.end(); ++it)
	{
		delete it->second;
	}
	s_instance->m_commands.clear();
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
	return s_instance;
}

void CommandFactory::destroyInstance()
{
	if (s_instance)
	{
		delete s_instance;
		s_instance = NULL;
	}
}

void CommandFactory::registerCommand(const std::string& name, ACommand* command)
{
	if (command)
		m_commands[name] = command;
}

ACommand* CommandFactory::getCommand(const std::string& name) const
{
	std::map< std::string, ACommand* >::const_iterator it = m_commands.find(name);

	if (it != m_commands.end())
		return it->second;

	return NULL;
}

bool CommandFactory::hasCommand(const std::string& name) const
{
	return m_commands.find(name) != m_commands.end();
}
