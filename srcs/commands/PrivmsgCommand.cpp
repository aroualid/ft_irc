#include "commands/PrivmsgCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "core/IChannel.hpp"
#include "core/IMessageBuffer.hpp"
#include "protocol/MessageParser.hpp"
#include "protocol/NumericReply.hpp"

REGISTER_COMMAND(PrivmsgCommand, irc::PRIVMSG, "PRIVMSG")

PrivmsgCommand::PrivmsgCommand(IServer& server) : ACommand(server)
{
}

PrivmsgCommand::~PrivmsgCommand()
{
}

void PrivmsgCommand::sendToTarget(IClient* client, const std::string& target,
								  const std::string& text)
{
	if (isChannelName(target))
	{
		IChannel* channel = m_server.getChannel(target);
		if (!channel)
		{
			sendReply(client, NumericReply::noSuchChannel(client->getNickname(), target));
			return;
		}

		if (!channel->hasMember(client))
		{
			sendReply(client, NumericReply::cannotSendToChan(client->getNickname(), target));
			return;
		}

		Message msg = buildMessage(client, target, text);
		broadcastToChannel(channel, client, msg);
	}
	else
	{
		IClient* targetClient = m_server.getClientByNickname(target);
		if (!targetClient)
		{
			sendReply(client, NumericReply::noSuchNick(client->getNickname(), target));
			return;
		}

		Message msg = buildMessage(client, target, text);
		sendToClient(targetClient, client, msg);
	}
}

void PrivmsgCommand::doExecute(IClient* client, const Message& message)
{
	const std::string& targets = message.m_params[0];
	const std::string& text = message.m_params[1];

	std::vector< std::string > targetList = splitByComma(targets);

	for (std::size_t i = 0; i < targetList.size(); i++)
	{
		if (!targetList[i].empty())
			sendToTarget(client, targetList[i], text);
	}
}

ACommand* PrivmsgCommand::create(IServer& server)
{
	return new (std::nothrow) PrivmsgCommand(server);
}
