#include "commands/PassCommand.hpp"
#include <new>

namespace
{
struct Registrar
{
	Registrar()
	{
		CommandFactory::getInstance()->registerCommandSpawner("PASS", &PassCommand::create);
	}
};
static Registrar g_registrar;
}; // namespace

PassCommand::PassCommand(IServer& server) : ACommand(server)
{
}

PassCommand::~PassCommand()
{
}

void PassCommand::execute(IClient* client, const Message& message)
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

ACommand* PassCommand::create(IServer& server)
{
	return new (std::nothrow) PassCommand(server);
}
