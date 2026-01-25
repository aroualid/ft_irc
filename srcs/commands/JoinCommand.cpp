#include "commands/JoinCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "core/IChannel.hpp"
#include "protocol/IrcUtils.hpp"
#include "protocol/MessageParser.hpp"
#include "protocol/NumericReply.hpp"

REGISTER_COMMAND(JoinCommand, irc::JOIN, "JOIN")

JoinCommand::JoinCommand(IServer& server) : ACommand(server)
{
}

JoinCommand::~JoinCommand()
{
}

void JoinCommand::doExecute(IClient* client, const Message& message)
{
	if (message.m_params.empty())
	{
		sendReply(client, NumericReply::needMoreParams(client->getNickname(), "JOIN"));
		return;
	}
	std::vector< std::string > channels = IrcUtils::splitByComma(message.m_params[0]);
	std::vector< std::string > keys;

	if (message.m_params.size() > 1)
		keys = IrcUtils::splitByComma(message.m_params[1]);

	for (std::size_t i = 0; i < channels.size(); i++)
	{
		std::string key = (i < keys.size()) ? keys[i] : "";
		joinSingleChannel(client, channels[i], key);
	}
}

void JoinCommand::joinSingleChannel(IClient* client, const std::string& channelName,
									const std::string& key)
{
	if (!IrcUtils::isValidChannelName(channelName))
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
			return;
		}
	}

	client->joinChannel(channel);

	Message joinMessage;
	joinMessage.m_prefix = client->getPrefix();
	joinMessage.m_command = getName();
	joinMessage.m_params.push_back(channel->getName());

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

ACommand* JoinCommand::create(IServer& server)
{
	return new (std::nothrow) JoinCommand(server);
}
