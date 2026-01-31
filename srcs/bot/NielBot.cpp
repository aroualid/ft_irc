#include "bot/NielBot.hpp"
#include "CommandType.hpp"
#include "Logger.hpp"
#include "bot/BotClient.hpp"
#include "bot/BotMessageBuffer.hpp"
#include "protocol/Message.hpp"
#include "protocol/MessageParser.hpp"

NielBot::NielBot(IServer& server, const std::string& nick) : m_server(server)
{
	m_client = new BotClient(nick, server);
	m_client->setBot(this);
	BotMessageBuffer* bmb = dynamic_cast< BotMessageBuffer* >(&m_client->getBuffer());
	bmb->setBot(this);
}

NielBot::~NielBot()
{
	delete m_client;
}

void NielBot::onChannelMessage(IClient* sender, IChannel* channel, const std::string& msg)
{
	if (sender == m_client)
		return;

	bool has42 = msg.find("42") != std::string::npos;
	if (has42)
	{
		std::ifstream file("ascii/xavier.txt");
		std::string line;
		while (std::getline(file, line))
			sendToChannel(channel, line);
	}
}

void NielBot::onPrivateMessage(IClient* sender, const std::string& msg)
{
	if (sender == m_client)
		return;
	(void)msg;
}

void NielBot::joinChannel(const std::string& channelName)
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

void NielBot::sendToChannel(IChannel* channel, const std::string& message)
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

IClient* NielBot::getClient()
{
	return (m_client);
}