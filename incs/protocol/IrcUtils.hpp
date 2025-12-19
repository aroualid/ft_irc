#pragma once

#include <string>

class IrcUtils
{
public:
	static bool iequals(const std::string& a, const std::string& b);

	static std::string toLower(const std::string& str);

	static char toLower(char c);
};
