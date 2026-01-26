#include "commands/TopicCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "protocol/MessageParser.hpp"
#include <new>

REGISTER_COMMAND(TopicCommand, irc::TOPIC, "TOPIC");

TopicCommand::TopicCommand(IServer& server) : ACommand(server)
{
}

TopicCommand::~TopicCommand()
{
}

void TopicCommand::doExecute(IClient* client, const Message& message)
{
	const std::string& channelName = message.m_params[0];

	IChannel* channel = m_server.getChannel(channelName);
	if (!channel)
	{
		sendReply(client, NumericReply::noSuchChannel(client->getNickname(), channelName));
		return;
	}

	if (!channel->hasMember(client))
	{
		sendReply(client, NumericReply::notOnChannel(client->getNickname(), channelName));
		return;
	}

	if (message.m_params.size() == 1)
	{
		const std::string& topic = channel->getTopic();
		if (topic.empty())
			sendReply(client, NumericReply::noTopic(client->getNickname(), channelName));
		else
			sendReply(client, NumericReply::topic(client->getNickname(), channelName, topic));
		return;
	}

	if (channel->isTopicRestricted() && !channel->isOperator(client))
	{
		sendReply(client, NumericReply::chanOpPrivsNeeded(client->getNickname(), channelName));
		return;
	}

	const std::string& newTopic = message.m_params[1];
	channel->setTopic(newTopic);

	Message topicMsg;
	topicMsg.m_prefix = client->getPrefix();
	topicMsg.m_command = "TOPIC";
	topicMsg.m_params.push_back(channelName);
	topicMsg.m_params.push_back(newTopic);

	channel->broadcast(MessageParser::serialize(topicMsg));
}

ACommand* TopicCommand::create(IServer& server)
{
	return new (std::nothrow) TopicCommand(server);
}
