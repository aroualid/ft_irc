#include "commands/JoinCommand.hpp"
#include "mock.hpp"
#include "protocol/MessageParser.hpp"
#include "protocol/NumericReply.hpp"
#include <cstddef>

JoinCommand::JoinCommand(Server& server) : ACommand(server)
{
}

JoinCommand::~JoinCommand()
{
}

bool JoinCommand::isValidChannelName(const std::string& name) const
{
	if (name.empty())
		return (false);
	if (name[0] != '#' && name[0] != '&')
		return (false);
	if (name.length() > 200)
		return (false);

	for (std::size_t i = 0; i < name.length(); i++)
	{
		char c = name[i];
		if (c == ' ' || c == ',' || c == '\07')
			return (false);
	}
	return (true);
}

void JoinCommand::execute(Client* client, const Message& message)
{
	if (!validateParamCount(client, message, minParams()))
		return;

	std::string channelName = message.m_params[0];

	if (!isValidChannelName(channelName))
	{
		sendReply(client, NumericReply::noSuchChannel(client->getNickname(), channelName));
		return;
	}

	if (client->isInChannel(channelName))
		return;

	Channel* channel = m_server.getChannel(channelName);
	if (!channel)
	{
		channel = m_server.createChannel(channelName, client);
		if (!channel)
		{
			sendReply(client, NumericReply::noSuchChannel(client->getNickname(), channelName));
			return;
		}
	}
	else
	{
		if (!channel->addMember(client))
		{
			// TODO: check modes if there's specific restrictions
			// so we can send an appropriate RPL :)
			return;
		}
	}
	client->joinChannel(channelName);

	Message joinMessage;
	joinMessage.m_prefix = client->getPrefix();
	joinMessage.m_command = getName();
	joinMessage.m_params.push_back(channelName);

	std::string serialized = MessageParser::serialize(joinMessage);
	channel->broadcast(serialized);

	if (!channel->getTopic().empty())
		sendReply(client,
				  NumericReply::topic(client->getNickname(), channelName, channel->getTopic()));
	else
		sendReply(client, NumericReply::noTopic(client->getNickname(), channelName));

	std::string memberList = channel->getMemberList();
	sendReply(client, NumericReply::namReply(client->getNickname(), channelName, memberList));
	sendReply(client, NumericReply::endOfNames(client->getNickname(), channelName));
}
