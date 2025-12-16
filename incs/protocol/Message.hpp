#pragma once

#include <string>
#include <vector>
struct Message
{
	std::string m_prefix;
	std::string m_command;
	std::vector< std::string > m_params;

	Message();
	bool isValid() const;
};
