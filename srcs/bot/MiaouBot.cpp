#include "bot/MiaouBot.hpp"
#include "CommandType.hpp"
#include "protocol/Message.hpp"
#include "protocol/MessageParser.hpp"
#include <ctime>
#include <cstdlib>


MiaouBot::MiaouBot(IServer& server, const std::string& nick) : m_server(server)
{
	m_client = new BotClient(nick, server);

}

MiaouBot::~MiaouBot()
{
	delete m_client;
}


void MiaouBot::onChannelMessage(IClient *sender, IChannel *channel, const std::string &message)
{
	(void) sender;
	(void) message;
	std::time_t start = std::time(NULL);
	while (true)
	{
		std::time_t now = std::time(NULL);
		double difftime = std::difftime(start, now);
		if (difftime >= 30.0)
		{
			int i = 1 + (rand()% 6);
			switch (i)
			{
				case 1:
					sendToChannel(channel, "MIAOUUUUUUUUU");
					break;
				case 2:
					sendToChannel(channel, "Quack i'm a DUCK NOW LET'S GOOO! 🦆🦆🦆🦆🦆");
					break;
				case 3:
					sendToChannel(channel, "Meow i'm a british cat 🇬🇧");
					break;
				case 4:
					sendToChannel(channel, "WAF im a weird cat");
					break;
				case 5:
					sendToChannel(channel, "ᵐᶦᵃᵒᵘ ᶦ ᵃᵐ ˢʰʸ ᵘʷᵘ ⁽,,>﹏<,,⁾👉👈");
					break;
				case 6:
					sendToChannel(channel, "check tes mails.");
					break;
				default:
					break;
			}
			start = std::time(NULL);
		}
	}
}

void MiaouBot::sendToChannel(IChannel *channel, const std::string &message)
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

void MiaouBot::joinChannel(const std::string& channelName)
{
	IChannel* channel = m_server.getChannel(channelName);
	if (!channel)
		channel = m_server.createChannel(channelName, m_client);
	if (!channel->hasMember(m_client))
	{
		channel->addMember(m_client);
		m_client->joinChannel(channel);
	}
}


void MiaouBot::onPrivateMessage(IClient* sender, const std::string& message)
{
	if (sender == m_client)
		return;
	(void)message;
}

IClient* MiaouBot::getClient()
{
	return (m_client);
}
