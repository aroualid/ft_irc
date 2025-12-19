#include "commands/UserCommand.hpp"
#include "protocol/NumericReply.hpp"

UserCommand::UserCommand(Server& server) : ACommand(server)
{
}

UserCommand::~UserCommand()
{
}

void UserCommand::execute(Client* client, const Message& message)
{
	if (!validateParamCount(client, message, minParams()))
		return;

	if (client->isRegistered())
	{
		sendReply(client, NumericReply::alreadyRegistered(client->getNickname()));
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

	client->setUsername(username);
	client->setRealname(realname);
}
