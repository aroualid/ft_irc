#include "commands/QuitCommand.hpp"
#include "IChannel.hpp"
#include "IClient.hpp"
#include "IServer.hpp"
#include "commands/ACommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "protocol/Message.hpp"
#include "protocol/MessageParser.hpp"
#include <new>
#include <set>
#include <string>

REGISTER_COMMAND(QuitCommand, irc::QUIT, "QUIT")

QuitCommand::QuitCommand(IServer& server) : ACommand(server)
{
}

QuitCommand::~QuitCommand()
{
}

void QuitCommand::broadcastQuitToChannels(IClient* client, const std::string& quitMessage)
{
	Message quitMsg;
	quitMsg.m_prefix = client->getPrefix();
	quitMsg.m_command = getName();

	if (!quitMessage.empty())
		quitMsg.m_params.push_back(quitMessage);

	std::string serialized = MessageParser::serialize(quitMsg);

	std::set< IClient* > notifiedClients;

	const std::set< std::string >& channels = client->getChannels();
	for (std::set< std::string >::const_iterator it = channels.begin(); it != channels.end(); ++it)
	{
		IChannel* channel = m_server.getChannel(*it);
		if (channel)
			channel->broadcast(serialized, client);
	}
}

void QuitCommand::doExecute(IClient* client, const Message& message)
{
	std::string quitMessage;

	if (message.m_params.empty() || message.m_params[0].empty())
		quitMessage = client->getNickname().empty() ? "" : client->getNickname();
	else
		quitMessage = message.m_params[0];

	broadcastQuitToChannels(client, quitMessage);

	const std::set< std::string >& channels = client->getChannels();
	std::set< std::string > channelsCopy = channels;

	for (std::set< std::string >::const_iterator it = channelsCopy.begin();
		 it != channelsCopy.end(); ++it)
	{
		IChannel* channel = m_server.getChannel(*it);
		if (channel)
		{
			channel->removeMember(client);
			client->leaveChannel(*it);
			m_server.deleteChannelIfEmpty(channel);
		}
	}
}

ACommand* QuitCommand::create(IServer& server)
{
	return new (std::nothrow) QuitCommand(server);
}
