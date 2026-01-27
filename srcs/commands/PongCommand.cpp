#include "commands/PongCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"

REGISTER_COMMAND(PongCommand, irc::PONG, "PONG");

PongCommand::PongCommand(IServer& server) : ACommand(server)
{
}

PongCommand::~PongCommand()
{
}

ACommand* PongCommand::create(IServer& server)
{
	return new PongCommand(server);
}

void PongCommand::doExecute(IClient* client, const Message& message)
{
	(void)message;
	client->setLastPingSent(0); // client is alive!! yay :D
}

void PongCommand::execute(IClient* client, const Message& message)
{
	if (!client)
		return;

	if (message.m_params.empty())
	{
		std::string nick = client->isRegistered() ? client->getNickname() : "*";
		NumericReply reply = NumericReply::noOrigin(nick);
		sendReply(client, reply);
		return;
	}

	doExecute(client, message);
}
