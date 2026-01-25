#include "commands/UserCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "protocol/NumericReply.hpp"

REGISTER_COMMAND(UserCommand, irc::USER, "USER")

UserCommand::UserCommand(IServer& server) : ACommand(server)
{
}

UserCommand::~UserCommand()
{
}

void UserCommand::doExecute(IClient* client, const Message& message)
{
	if (client->isRegistered())
	{
		sendReply(client, NumericReply::alreadyRegistered(client->getNickname()));
		return;
	}

	if (message.m_params.size() < 4)
	{
		sendReply(client,
				  NumericReply::needMoreParams(
					  client->getNickname().empty() ? "*" : client->getNickname(), getName()));
		return;
	}

	const std::string& username = message.m_params[0];
	const std::string& realname = message.m_params[3];

	if (username.empty())
	{
		sendReply(client,
				  NumericReply::needMoreParams(
					  client->getNickname().empty() ? "*" : client->getNickname(), getName()));
		return;
	}

	bool wasRegistered = client->isRegistered();
	client->setUsername(username);
	client->setRealname(realname);

	if (!wasRegistered && client->isRegistered())
	{
		sendReply(client, NumericReply::welcome(client->getNickname()));
	}
}

ACommand* UserCommand::create(IServer& server)
{
	return new (std::nothrow) UserCommand(server);
}
