#include "protocol/IrcUtils.hpp"
#include <cctype>

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
