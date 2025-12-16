#include "protocol/MessageParser.hpp"
#include <cstddef>
#include <sstream>
#include <string>

Message MessageParser::parse(const std::string& raw)
{
	Message message;

	if (raw.empty())
		return (message);

	std::string line = raw;

	if (line.size() >= 2 && line.substr(line.size() - 2) == "\r\n")
		line = line.substr(0, line.size() - 2);
	else if (!line.empty() && line[line.size() - 1] == '\n')
		line = line.substr(0, line.size() - 1);

	if (line.empty())
		return (message);
	if (line.size() > 510)
		return (message);

	std::stringstream iss(line);
	std::string word;

	if (line[0] == ':')
	{
		iss >> message.m_prefix;
		message.m_prefix = message.m_prefix.substr(1);
	}

	if (!(iss >> message.m_command))
		return (Message());

	for (size_t i = 0; i < message.m_command.size(); i++)
		message.m_command[i] = std::toupper(message.m_command[i]);

	while (iss >> word)
	{
		if (message.m_params.size() >= 15)
			break;

		if (word[0] == ':')
		{
			std::string trail = word.substr(1);
			std::string rest;

			if (std::getline(iss, rest))
				if (!rest.empty())
					trail += rest;
			message.m_params.push_back(trail);
			break;
		}
		else
			message.m_params.push_back(word);
	}

	return (message);
}

std::string MessageParser::serialize(const Message& message)
{
	if (message.m_command.empty())
		return "";

	std::string result;

	if (!message.m_prefix.empty())
	{
		result += ":";
		result += message.m_prefix;
		result += " ";
	}

	result += message.m_command;

	for (size_t i = 0; i < message.m_params.size(); i++)
	{
		result += " ";

		if (i == message.m_params.size() - 1 &&
			(message.m_params[i].find(' ') != std::string::npos || message.m_params[i].empty()))
		{
			result += ":";
			result += message.m_params[i];
		}
		else
			result += message.m_params[i];
	}
	result += "\r\n";
	return (result);
}

bool MessageParser::isValid(const std::string& raw)
{
	if (raw.empty() || raw.size() < 2 || raw.size() > 512)
		return (false);

	bool has_valid_ending = false;

	if (raw.size() >= 2 && raw.substr(raw.size() - 2) == "\r\n")
		has_valid_ending = true;
	else if (raw[raw.size() - 1] == '\n')
		has_valid_ending = true;

	if (!has_valid_ending)
		return (false);

	Message message = parse(raw);
	return (message.isValid());
}
