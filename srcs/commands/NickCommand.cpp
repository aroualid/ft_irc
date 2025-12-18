#include "commands/NickCommand.hpp"
#include "commands/ACommand.hpp"
#include <cctype>

NickCommand::NickCommand(Server* server) : ACommand(server)
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
			c == '^' || c == '{' || c == '}')
			continue;
		return (false);
	}
	return (true);
}

void NickCommand::execute(Client* client, const Message& message)
{
	if (!validateParamCount(client, message, minParams()))
		return;

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

	Client* existing_client = m_server->getClientByNickname(nickname);
	if (existing_client && existing_client != client)
	{
		sendReply(client,
				  NumericReply::nicknameInUse(
					  client->getNickname().empty() ? "*" : client->getNickname(), nickname));
		return;
	}

	bool isNicknameChange = !client->getNickname().empty();

	if (isNicknameChange && client->isRegistered())
	{
		// TODO: broadcast nickname change to all currently connected clients
		// this will come during next milestone
	}

	if (client->isRegistered())
	{
		sendReply(client, NumericReply::welcome(client->getNickname()));
		// TODO: probably add more info (aka RPL_YOURHOST, RPL_CREATED and RPL_MYINFO)
	}
}
