#include "commands/NickCommand.hpp"
#include "commands/ACommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "core/IChannel.hpp"
#include "core/IMessageBuffer.hpp"
#include "protocol/IrcUtils.hpp"
#include "protocol/MessageParser.hpp"
#include "protocol/NumericReply.hpp"

REGISTER_COMMAND(NickCommand, irc::NICK, "NICK")

NickCommand::NickCommand(IServer& server) : ACommand(server)
{
}

NickCommand::~NickCommand()
{
}

void NickCommand::doExecute(IClient* client, const Message& message)
{
	if (message.m_params.empty())
	{
		sendReply(client, NumericReply::noNicknameGiven(
							  client->getNickname().empty() ? "*" : client->getNickname()));
		return;
	}
	const std::string& nickname = message.m_params[0];
	if (nickname.empty())
	{
		sendReply(client, NumericReply::noNicknameGiven(
							  client->getNickname().empty() ? "*" : client->getNickname()));
		return;
	}

	if (!IrcUtils::isValidNickname(nickname))
	{
		sendReply(client,
				  NumericReply::erroneusNickname(
					  client->getNickname().empty() ? "*" : client->getNickname(), nickname));
		return;
	}

	IClient* existing_client = m_server.getClientByNickname(nickname);
	if (existing_client && existing_client != client)
	{
		sendReply(client,
				  NumericReply::nicknameInUse(
					  client->getNickname().empty() ? "*" : client->getNickname(), nickname));
		return;
	}

	bool wasRegistered = client->isRegistered();
	bool isNicknameChange = wasRegistered && !client->getNickname().empty();
	std::string oldPrefix;

	if (isNicknameChange)
	{
		oldPrefix = client->getPrefix();
		m_server.unregisterClient(client->getNickname());
	}

	client->setNickname(nickname);

	if (!wasRegistered && client->isRegistered())
	{
		sendReply(client, NumericReply::welcome(client->getNickname()));
	}
	else if (isNicknameChange)
	{
		Message nickMessage;
		nickMessage.m_prefix = oldPrefix;
		nickMessage.m_command = "NICK";
		nickMessage.m_params.push_back(nickname);
		std::string serialized = MessageParser::serialize(nickMessage);

		client->getBuffer().appendWrite(serialized);

		const std::set< IChannel* >& channels = client->getChannels();
		for (std::set< IChannel* >::const_iterator it = channels.begin(); it != channels.end();
			 ++it)
		{
			(*it)->broadcast(serialized, client);
		}
	}
}

ACommand* NickCommand::create(IServer& server)
{
	return new (std::nothrow) NickCommand(server);
}
