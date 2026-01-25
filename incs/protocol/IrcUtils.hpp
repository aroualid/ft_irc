#pragma once

#include <string>
#include <vector>

class IrcUtils
{
public:
	static bool iequals(const std::string& a, const std::string& b);

	static std::string toLower(const std::string& str);

	static char toLower(char c);

	// RFC 1459 compliant nickname validation
	// Max 9 chars, starts with letter, allows: a-zA-Z0-9-[]\^{}|`
	static bool isValidNickname(const std::string& nick);

	// RFC 1459 compliant channel name validation
	// Starts with # or &, max 200 chars, no spaces/^G/comma
	static bool isValidChannelName(const std::string& name);

	// Split a comma-separated string into a vector
	static std::vector< std::string > splitByComma(const std::string& str);
};
