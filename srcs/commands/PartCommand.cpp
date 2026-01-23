#include "commands/PartCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "core/IChannel.hpp"
#include "protocol/MessageParser.hpp"
#include "protocol/NumericReply.hpp"

REGISTER_COMMAND(PartCommand, irc::PART, "PART")

PartCommand::PartCommand(IServer& server) : ACommand(server)
{
}

PartCommand::~PartCommand()
{
}

void PartCommand::doExecute(IClient* client, const Message& message)
{
	std::string channelName = message.m_params[0];
	std::string partMessage = message.m_params.size() > 1 ? message.m_params[1] : "";

	IChannel* channel = m_server.getChannel(channelName);

	if (!channel)
	{
		sendReply(client, NumericReply::noSuchChannel(client->getNickname(), channelName));
		return;
	}

	if (!client->isInChannel(channelName))
	{
		sendReply(client, NumericReply::notOnChannel(client->getNickname(), channelName));
		return;
	}

	Message partMsg;
	partMsg.m_prefix = client->getPrefix();
	partMsg.m_command = getName();
	partMsg.m_params.push_back(channelName);
	if (!partMessage.empty())
		partMsg.m_params.push_back(partMessage);

	std::string serialized = MessageParser::serialize(partMsg);
	channel->broadcast(serialized);

	channel->removeMember(client);
	client->leaveChannel(channel);

	m_server.deleteChannelIfEmpty(channel);
}

ACommand* PartCommand::create(IServer& server)
{
	return new (std::nothrow) PartCommand(server);
}
