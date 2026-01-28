#include "bot/SixSevenBot.hpp"
#include "CommandType.hpp"
#include "protocol/Message.hpp"
#include "protocol/MessageParser.hpp"

SixSevenBot::SixSevenBot(IServer& server, const std::string& nick) : m_server(server)
{
	(void)nick;
}

void SixSevenBot::onChannelMessage(IClient *sender, IChannel *channel, const std::string &message)
{
	if (sender == m_client)
		return ;

	bool hasSix = message.find('6') != std::string::npos;
	bool hasSeven = message.find('7') != std::string::npos;
	bool isValidSixSeven = (hasSix && hasSeven) && message.find('6') < message.find('7');

	if (isValidSixSeven)
		sendToChannel(channel, "DID SOMEONE SAY... SIX SEVENNNNNNNN????");
}

void SixSevenBot::sendToChannel(IChannel *channel, const std::string &message)
{
	Message msg;
	msg.m_prefix = m_client->getPrefix();
	msg.m_command = "PRIVMSG";
	msg.m_command_type = irc::PRIVMSG;
	msg.m_params.push_back(channel->getName());
	msg.m_params.push_back(message);

	std::string serialized = MessageParser::serialize(msg);
	channel->broadcast(serialized, m_client);
}
