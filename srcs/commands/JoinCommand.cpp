#include "commands/JoinCommand.hpp"

JoinCommand::JoinCommand(Server& server) : ACommand(server)
{
}

JoinCommand::~JoinCommand()
{
}

void JoinCommand::execute(Client* client, const Message& message)
{
	if (!validateParamCount(client, message, minParams()))
		return;
}
