#include "commands/NamesCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "core/IChannel.hpp"
#include <new>

REGISTER_COMMAND(NamesCommand, irc::NAMES, "NAMES");

NamesCommand::NamesCommand(IServer& server) : ACommand(server)
{
}

NamesCommand::~NamesCommand()
{
}

void NamesCommand::doExecute(IClient* client, const Message& message)
{
	const std::string& channelName = message.m_params[0];

	IChannel* channel = m_server.getChannel(channelName);
	if (!channel)
	{
		sendReply(client, NumericReply::endOfNames(client->getNickname(), channelName));
		return;
	}

	std::string memberList = channel->getMemberList();
	sendReply(client, NumericReply::namReply(client->getNickname(), channelName, memberList));
	sendReply(client, NumericReply::endOfNames(client->getNickname(), channelName));
}

ACommand* NamesCommand::create(IServer& server)
{
	return new (std::nothrow) NamesCommand(server);
}
