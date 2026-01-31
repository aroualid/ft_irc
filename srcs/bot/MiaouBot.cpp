#include "bot/MiaouBot.hpp"
#include "CommandType.hpp"
#include "protocol/Message.hpp"
#include "protocol/MessageParser.hpp"
#include <ctime>
#include <cstdlib>



MiaouBot::MiaouBot(IServer& server, const std::string& nick) : m_server(server), _count_message(0), _index(0)
{
	m_client = new BotClient(nick, server);
	m_client->setBot(this);
	BotMessageBuffer *bmb = dynamic_cast<BotMessageBuffer *>(&m_client->getBuffer());
	bmb->setBot(this);
	std::srand(std::time(0));
}

MiaouBot::~MiaouBot()
{
	delete m_client;
}


void MiaouBot::onChannelMessage(IClient *sender, IChannel *channel, const std::string &message)
{

	if (sender == m_client)
		return;
	if (message.empty())
        return;
	_count_message++;

	if (_count_message >= 5)
	{
		_count_message = 0;
		_index++;

		const char* responses[] = {
            "MIAOUUUUUUUUU",
            "Quack i'm a DUCK NOW LET'S GOOO! 🦆🦆🦆🦆",
            "Meow i'm a british cat 🇬🇧 ",
            "WAF im a weird cat",
            "miaou I am shy uwu (,,>﹏<,,)👉👈",
            "check tes mails."
		};

        sendToChannel(channel, responses[_index%6]);
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
