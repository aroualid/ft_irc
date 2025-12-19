#include "commands/PassCommand.hpp"

PassCommand::PassCommand(Server& server) : ACommand(server)
{
}

PassCommand::~PassCommand()
{
}

void PassCommand::execute(Client* client, const Message& message)
{
	if (!validateParamCount(client, message, minParams()))
		return;

	if (client->isRegistered())
	{
		sendReply(client, NumericReply::alreadyRegistered(client->getNickname()));
		return;
	}

	const std::string& providedPass = message.m_params[0];

	if (providedPass == m_server.getPassword())
		client->setPasswordProvided(true);
}
