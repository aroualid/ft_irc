#include "commands/PassCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include <new>

REGISTER_COMMAND(PassCommand, irc::PASS, "PASS")

PassCommand::PassCommand(IServer& server) : ACommand(server)
{
}

PassCommand::~PassCommand()
{
}

void PassCommand::doExecute(IClient* client, const Message& message)
{
	if (client->isRegistered())
	{
		sendReply(client, NumericReply::alreadyRegistered(client->getNickname()));
		return;
	}

	const std::string& providedPass = message.m_params[0];

	if (providedPass == m_server.getPassword())
		client->setPasswordProvided(true);
	else
		client->setPasswordProvided(false);
}

ACommand* PassCommand::create(IServer& server)
{
	return new (std::nothrow) PassCommand(server);
}
