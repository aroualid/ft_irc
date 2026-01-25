#include "protocol/IrcUtils.hpp"
#include <cctype>
#include <cstddef>

char IrcUtils::toLower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + ('a' - 'A');

	switch (c)
	{
	case '[':
		return '{';
	case ']':
		return '}';
	case '\\':
		return '|';
	default:
		return c;
	}
}

std::string IrcUtils::toLower(const std::string& str)
{
	std::string result;
	result.reserve(str.size());

	for (size_t i = 0; i < str.size(); i++)
		result += toLower(str[i]);

	return result;
}

bool IrcUtils::iequals(const std::string& a, const std::string& b)
{
	if (a.size() != b.size())
		return false;

	for (size_t i = 0; i < a.size(); i++)
	{
		if (toLower(a[i]) != toLower(b[i]))
			return false;
	}

	return true;
}

bool IrcUtils::isValidNickname(const std::string& nick)
{
	if (nick.empty() || nick.length() > 9 || !std::isalpha(nick[0]))
		return false;

	for (std::size_t i = 1; i < nick.size(); i++)
	{
		char c = nick[i];
		if (std::isalnum(c) || c == '-' || c == '[' || c == ']' || c == '\\' || c == '`' ||
			c == '^' || c == '{' || c == '}' || c == '|')
			continue;
		return false;
	}
	return true;
}

bool IrcUtils::isValidChannelName(const std::string& name)
{
	if (name.empty())
		return false;
	if (name[0] != '#' && name[0] != '&')
		return false;
	if (name.length() > 200)
		return false;

	for (std::size_t i = 0; i < name.length(); i++)
	{
		char c = name[i];
		if (c == ' ' || c == ',' || c == '\07')
			return false;
	}
	return true;
}

std::vector< std::string > IrcUtils::splitByComma(const std::string& str)
{
	std::vector< std::string > result;
	std::string current;

	for (std::size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == ',')
		{
			if (!current.empty())
			{
				result.push_back(current);
				current.clear();
			}
		}
		else
		{
			current += str[i];
		}
	}

	if (!current.empty())
		result.push_back(current);

	return result;
}
