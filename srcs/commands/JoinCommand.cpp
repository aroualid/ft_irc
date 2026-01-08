#include "commands/JoinCommand.hpp"
#include "core/IChannel.hpp"
#include "protocol/MessageParser.hpp"
#include "protocol/NumericReply.hpp"
#include <cstddef>

JoinCommand::JoinCommand(IServer& server) : ACommand(server)
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

std::vector< std::string > JoinCommand::splitByComma(const std::string& str) const
{
	std::vector< std::string > result;
	std::string current;

	for (std::size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == ',')
		{
			if (!current.empty())
			{
				result.push_back(current);
				current.clear();
			}
		}
		else
		{
			current += str[i];
		}
	}

	// Don't forget the last element
	if (!current.empty())
		result.push_back(current);

	return result;
}

void JoinCommand::execute(IClient* client, const Message& message)
{
	if (!validateParamCount(client, message, minParams()))
		return;

	std::vector< std::string > channels = splitByComma(message.m_params[0]);
	std::vector< std::string > keys;

	if (message.m_params.size() > 1)
		keys = splitByComma(message.m_params[1]);

	for (std::size_t i = 0; i < channels.size(); i++)
	{
		std::string key = (i < keys.size()) ? keys[i] : "";
		joinSingleChannel(client, channels[i], key);
	}
}

void JoinCommand::joinSingleChannel(IClient* client, const std::string& channelName,
									const std::string& key)
{
	if (!isValidChannelName(channelName))
	{
		sendReply(client, NumericReply::noSuchChannel(client->getNickname(), channelName));
		return;
	}

	if (client->isInChannel(channelName))
		return;

	IChannel* channel = m_server.getChannel(channelName);
	bool isNewChannel = (channel == NULL);

	if (isNewChannel)
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
		if (!channel->addMember(client, key))
		{
			if (channel->isInviteOnly() && !channel->isInvited(client))
				sendReply(client, NumericReply::inviteOnlyChan(client->getNickname(), channelName));
			else if (channel->getUserLimit() > 0 &&
					 static_cast< int >(channel->getMemberCount()) >= channel->getUserLimit())
				sendReply(client, NumericReply::channelIsFull(client->getNickname(), channelName));
			else if (!channel->getKey().empty() && key != channel->getKey())
				sendReply(client, NumericReply::badChannelKey(client->getNickname(), channelName));
			else
				sendReply(client, NumericReply::inviteOnlyChan(client->getNickname(), channelName));
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
