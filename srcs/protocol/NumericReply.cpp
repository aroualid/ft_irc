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

NumericReply NumericReply::noTopic(const std::string& nick, const std::string& channel)
{
	std::string message = channel + " :No topic set";
	return NumericReply(331, nick, message);
}

NumericReply NumericReply::topic(const std::string& nick, const std::string& channel,
								 const std::string& topic)
{
	std::string message = channel + " :" + topic;
	return NumericReply(332, nick, message);
}

NumericReply NumericReply::namReply(const std::string& nick, const std::string& channel,
									const std::string& names)
{
	std::string message = "= " + channel + " :" + names;
	return NumericReply(353, nick, message);
}

NumericReply NumericReply::endOfNames(const std::string& nick, const std::string& channel)
{
	std::string message = channel + " :End of /NAMES list";
	return NumericReply(366, nick, message);
}

NumericReply NumericReply::noSuchChannel(const std::string& nick, const std::string& channel)
{
	std::string message = channel + " :No such channel";
	return NumericReply(403, nick, message);
}

NumericReply NumericReply::channelIsFull(const std::string& nick, const std::string& channel)
{
	std::string message = channel + " :Cannot join channel (+l)";
	return NumericReply(471, nick, message);
}

NumericReply NumericReply::inviteOnlyChan(const std::string& nick, const std::string& channel)
{
	std::string message = channel + " :Cannot join channel (+i)";
	return NumericReply(473, nick, message);
}

NumericReply NumericReply::badChannelKey(const std::string& nick, const std::string& channel)
{
	std::string message = channel + " :Cannot join channel (+k)";
	return NumericReply(475, nick, message);
}

NumericReply NumericReply::noSuchNick(const std::string& nick, const std::string& target)
{
	std::string message = target + " :No such nick/channel";
	return NumericReply(401, nick, message);
}

NumericReply NumericReply::noRecipient(const std::string& nick, const std::string& command)
{
	std::string message = ":No recipient given (" + command + ")";
	return NumericReply(411, nick, message);
}

NumericReply NumericReply::noTextToSend(const std::string& nick)
{
	return NumericReply(412, nick, ":No text to send");
}

NumericReply NumericReply::cannotSendToChan(const std::string& nick, const std::string& channel)
{
	std::string message = channel + " :Cannot send to channel";
	return NumericReply(404, nick, message);
}

NumericReply NumericReply::notOnChannel(const std::string& nick, const std::string& channel)
{
	std::string message = channel + " :You're not on that channel";
	return NumericReply(442, nick, message);
}

NumericReply NumericReply::noOrigin(const std::string& nick)
{
	std::string message = ":No origin specified";
	return NumericReply(409, nick, message);
}

NumericReply NumericReply::notRegistered(const std::string& nick)
{
	return NumericReply(451, nick.empty() ? "*" : nick, ":You have not registered");
}

NumericReply NumericReply::userNotInChannel(const std::string& nick, const std::string& target,
											const std::string& channel)
{
	std::string message = target + " " + channel + " :They aren't on that channel";
	return NumericReply(441, nick, message);
}

NumericReply NumericReply::chanOpPrivsNeeded(const std::string& nick, const std::string& channel)
{
	std::string message = channel + " :You're not channel operator (boooo!)";
	return NumericReply(482, nick, message);
}
