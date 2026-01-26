#include "commands/WhoCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "core/IChannel.hpp"
#include "core/IClient.hpp"
#include <new>
#include <vector>

REGISTER_COMMAND(WhoCommand, irc::WHO, "WHO");

WhoCommand::WhoCommand(IServer& server) : ACommand(server)
{
}

WhoCommand::~WhoCommand()
{
}

void WhoCommand::doExecute(IClient* client, const Message& message)
{
	const std::string& target = message.m_params[0];

	if (target[0] == '#' || target[0] == '&')
	{
		IChannel* channel = m_server.getChannel(target);
		if (!channel)
		{
			sendReply(client, NumericReply::endOfWho(client->getNickname(), target));
			return;
		}

		std::vector< IClient* > members = channel->getMembers();
		for (std::vector< IClient* >::iterator it = members.begin(); it != members.end(); ++it)
		{
			IClient* member = *it;
			std::string flags = "H";
			if (channel->isOperator(member))
				flags += "@";

			sendReply(client, NumericReply::whoReply(
								  client->getNickname(), channel->getName(), member->getUsername(),
								  member->getHostname(), m_server.getServerName(),
								  member->getNickname(), flags, member->getRealname()));
		}
		sendReply(client, NumericReply::endOfWho(client->getNickname(), target));
	}
	else
	{
		IClient* targetClient = m_server.getClientByNickname(target);
		if (targetClient)
		{
			sendReply(client, NumericReply::whoReply(
								  client->getNickname(), "*", targetClient->getUsername(),
								  targetClient->getHostname(), m_server.getServerName(),
								  targetClient->getNickname(), "H", targetClient->getRealname()));
		}
		sendReply(client, NumericReply::endOfWho(client->getNickname(), target));
	}
}

ACommand* WhoCommand::create(IServer& server)
{
	return new (std::nothrow) WhoCommand(server);
}
