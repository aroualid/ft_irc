#pragma once

#include "commands/CommandType.hpp"
#include <string>
#include <vector>
struct Message
{
	std::string m_prefix;
	std::string m_command;
	std::vector< std::string > m_params;
	irc::CommandType m_command_type;

	Message();
	bool isValid() const;
};
