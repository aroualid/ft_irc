#include "commands/PingCommand.hpp"
#include "commands/ACommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "core/IMessageBuffer.hpp"
#include "protocol/Message.hpp"
#include "protocol/MessageParser.hpp"
#include <new>

REGISTER_COMMAND(PingCommand, irc::PING, "PING")

PingCommand::PingCommand(IServer& server) : ACommand(server)
{
}

PingCommand::~PingCommand()
{
}

void PingCommand::doExecute(IClient* client, const Message& message)
{
	std::string origin = message.m_params[0];
	std::string serverName = m_server.getServerName();

	Message pongResponse;
	pongResponse.m_prefix = serverName;
	pongResponse.m_command = "PONG";
	pongResponse.m_params.push_back(serverName);
	pongResponse.m_params.push_back(origin);

	std::string serialized = MessageParser::serialize(pongResponse);
	client->getBuffer().appendWrite(serialized);
}

void PingCommand::execute(IClient* client, const Message& message)
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

ACommand* PingCommand::create(IServer& server)
{
	return new (std::nothrow) PingCommand(server);
}
