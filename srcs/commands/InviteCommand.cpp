#include "commands/InviteCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "core/IMessageBuffer.hpp"
#include "protocol/MessageParser.hpp"
#include <iostream>
#include <new>

REGISTER_COMMAND(InviteCommand, irc::INVITE, "INVITE");

InviteCommand::InviteCommand(IServer& server) : ACommand(server)
{
}

InviteCommand::~InviteCommand()
{
}

void InviteCommand::doExecute(IClient* client, const Message& message)
{
	const std::string& targetNick = message.m_params[0];
	const std::string& channelName = message.m_params[1];

	IClient* target = m_server.getClientByNickname(targetNick);
	if (!target)
	{
		sendReply(client, NumericReply::noSuchNick(client->getNickname(), targetNick));
		return;
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

	if (channel->isInviteOnly() && !channel->isOperator(client))
	{
		sendReply(client, NumericReply::chanOpPrivsNeeded(client->getNickname(), channelName));
		return;
	}

	if (channel->hasMember(target))
	{
		sendReply(client,
				  NumericReply::userOnChannel(client->getNickname(), targetNick, channelName));
		return;
	}

	channel->addInvite(target);

	sendReply(client, NumericReply::inviting(client->getNickname(), targetNick, channelName));

	Message inviteMsg;
	inviteMsg.m_prefix = client->getPrefix();
	inviteMsg.m_command = "INVITE";
	inviteMsg.m_params.push_back(targetNick);
	inviteMsg.m_params.push_back(channelName);

	target->getBuffer().appendWrite(MessageParser::serialize(inviteMsg));
}

ACommand* InviteCommand::create(IServer& server)
{
	return new (std::nothrow) InviteCommand(server);
}
