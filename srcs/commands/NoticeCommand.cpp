#include "commands/NoticeCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "core/IChannel.hpp"
#include "core/IMessageBuffer.hpp"
#include "protocol/MessageParser.hpp"

REGISTER_COMMAND(NoticeCommand, irc::NOTICE, "NOTICE")

NoticeCommand::NoticeCommand(IServer& server) : ACommand(server)
{
}

NoticeCommand::~NoticeCommand()
{
}

void NoticeCommand::execute(IClient* client, const Message& message)
{
	if (!client || message.m_params.size() < minParams())
		return;

	doExecute(client, message);
}

void NoticeCommand::sendToTarget(IClient* client, const std::string& target,
								 const std::string& text)
{
	if (isChannelName(target))
	{
		IChannel* channel = m_server.getChannel(target);
		if (!channel)
			return;

		if (!channel->hasMember(client))
			return;

		Message msg = buildMessage(client, target, text);
		broadcastToChannel(channel, client, msg);
	}
	else
	{
		IClient* targetClient = m_server.getClientByNickname(target);
		if (!targetClient)
			return;

		Message msg = buildMessage(client, target, text);
		sendToClient(targetClient, client, msg);
	}
}

void NoticeCommand::doExecute(IClient* client, const Message& message)
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

ACommand* NoticeCommand::create(IServer& server)
{
	return new (std::nothrow) NoticeCommand(server);
}
