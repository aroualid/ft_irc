#include "protocol/NumericReply.hpp"
#include <iomanip>
#include <sstream>

NumericReply::NumericReply(int code, const std::string& target, const std::string& message)
	: m_code(code), m_target(target), m_message(message)
{
}

std::string NumericReply::toString() const
{
	std::ostringstream oss;

	oss << std::setfill('0') << std::setw(3) << m_code;
	oss << " " << m_target;
	oss << " " << m_message;
	oss << "\r\n";

	return (oss.str());
}

NumericReply NumericReply::welcome(const std::string& nick)
{
	std::string message = "Welcome to 42{83400_61500} (aka ft_irchouine), " + nick + " :D";
	return NumericReply(1, nick, message);
}

NumericReply NumericReply::noNicknameGiven(const std::string& nick)
{
	return NumericReply(431, nick.empty() ? "*" : nick, ":No nickname given");
}

NumericReply NumericReply::erroneusNickname(const std::string& nick, const std::string& bad_nick)
{
	std::string message = bad_nick + " :Erroneous nickname";
	return NumericReply(432, nick.empty() ? "*" : nick, message);
}

NumericReply NumericReply::nicknameInUse(const std::string& nick, const std::string& bad_nick)
{
	std::string message = bad_nick + " :Nickname is already in use";
	return NumericReply(433, nick.empty() ? "*" : nick, message);
}

NumericReply NumericReply::needMoreParams(const std::string& nick, const std::string& command)
{
	std::string message = command + " :Not enough parameters";
	return NumericReply(461, nick.empty() ? "*" : nick, message);
}

NumericReply NumericReply::alreadyRegistered(const std::string& nick)
{
	return NumericReply(462, nick, ":You may not reregister");
}

NumericReply NumericReply::passwordMismatch(const std::string& nick)
{
	return NumericReply(464, nick.empty() ? "*" : nick, ":Password incorrect");
}
