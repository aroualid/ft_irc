#include "commands/KickCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "protocol/MessageParser.hpp"
#include <new>

REGISTER_COMMAND(KickCommand, irc::KICK, "KICK");

KickCommand::KickCommand(IServer& server) : ACommand(server)
{
}

KickCommand::~KickCommand()
{
}

void KickCommand::doExecute(IClient* client, const Message& message)
{
	const std::string& channelName = message.m_params[0];
	const std::string& targetNick = message.m_params[1];

	std::string reason = "Kicked (they did not like you)";
	if (message.m_params.size() > 2)
	{
		reason = message.m_params[2];
		if (!reason.empty() && reason.find(' ') == std::string::npos)
			reason += " ";
	}

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

	if (!channel->isOperator(client))
	{
		sendReply(client, NumericReply::chanOpPrivsNeeded(client->getNickname(), channelName));
		return;
	}

	IClient* target = channel->getMemberByNickname(targetNick);
	if (!target || !channel->hasMember(target))
	{
		sendReply(client,
				  NumericReply::userNotInChannel(client->getNickname(), targetNick, channelName));
		return;
	}

	Message kickMsg;
	kickMsg.m_prefix = client->getPrefix();
	kickMsg.m_command = "KICK";
	kickMsg.m_params.push_back(channelName);
	kickMsg.m_params.push_back(targetNick);
	kickMsg.m_params.push_back(reason);

	channel->broadcast(MessageParser::serialize(kickMsg));
	channel->removeMember(target);
	target->leaveChannel(channel);

	if (channel->isEmpty())
		m_server.deleteChannelIfEmpty(channel);
}

ACommand* KickCommand::create(IServer& server)
{
	return new (std::nothrow) KickCommand(server);
}
