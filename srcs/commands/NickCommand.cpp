#include "commands/NickCommand.hpp"
#include "commands/ACommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "protocol/NumericReply.hpp"
#include <cctype>

REGISTER_COMMAND(NickCommand, irc::NICK, "NICK")

NickCommand::NickCommand(IServer& server) : ACommand(server)
{
}

NickCommand::~NickCommand()
{
}

bool NickCommand::isValidNickname(const std::string& nickname) const
{
	if (nickname.empty() || nickname.length() > 9 || !std::isalpha(nickname[0]))
		return (false);

	for (size_t i = 1; i < nickname.size(); i++)
	{
		char c = nickname[i];
		if (std::isalnum(c) || c == '-' || c == '[' || c == ']' || c == '\\' || c == '`' ||
			c == '^' || c == '{' || c == '}' || c == '|')
			continue;
		return (false);
	}
	return (true);
}

void NickCommand::doExecute(IClient* client, const Message& message)
{
	const std::string& nickname = message.m_params[0];
	if (nickname.empty())
	{
		sendReply(client, NumericReply::noNicknameGiven(
							  client->getNickname().empty() ? "*" : client->getNickname()));
		return;
	}

	if (!isValidNickname(nickname))
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
	bool isNicknameChange = !client->getNickname().empty();
	client->setNickname(nickname);

	if (!wasRegistered && client->isRegistered())
	{
		sendReply(client, NumericReply::welcome(client->getNickname()));
	}
	else if (isNicknameChange && client->isRegistered())
	{
		// TODO: broadcast nickname change to all channels this user is in
		// this will come during milestone 3
	}
}

ACommand* NickCommand::create(IServer& server)
{
	return new (std::nothrow) NickCommand(server);
}
