#include "commands/ACommand.hpp"
#include "core/IMessageBuffer.hpp"

ACommand::ACommand(IServer& server) : m_server(server)
{
}

ACommand::~ACommand()
{
}

void ACommand::sendReply(IClient* client, const NumericReply& reply)
{
	if (!client)
		return;

	std::string replyStr = reply.toString();
	client->getBuffer().appendWrite(replyStr);
}

bool ACommand::validateParamCount(IClient* client, const Message& message, size_t min)
{
	if (message.m_params.size() < min)
	{
		std::string nick = "*";
		if (client && client->isRegistered())
			nick = client->getNickname();

		NumericReply reply = NumericReply::needMoreParams(nick, message.m_command);
		sendReply(client, reply);

		return false;
	}

	return true;
}
