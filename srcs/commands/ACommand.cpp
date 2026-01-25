#include "commands/ACommand.hpp"
#include "core/IChannel.hpp"
#include "core/IClient.hpp"
#include "core/IMessageBuffer.hpp"
#include "network/MessageBuffer.hpp"
#include "protocol/IrcUtils.hpp"
#include "protocol/MessageParser.hpp"

#include <vector>

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

std::vector< std::string > ACommand::splitByComma(const std::string& str)
{
	std::vector< std::string > result;
	std::string current;

	for (std::size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == ',')
		{
			if (!current.empty())
			{
				result.push_back(current);
				current.clear();
			}
		}
		else
		{
			current += str[i];
		}
	}

	if (!current.empty())
		result.push_back(current);

	return result;
}

bool ACommand::isChannelName(const std::string& name)
{
	return IrcUtils::isValidChannelName(name);
}

void ACommand::execute(IClient* client, const Message& message)
{
	if (!client)
		return;

	if (requiresRegistration() && !client->isRegistered())
	{
		std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
		NumericReply reply = NumericReply::notRegistered(nick);
		sendReply(client, reply);
		return;
	}

	if (!validateParamCount(client, message, minParams()))
		return;

	doExecute(client, message);
}

Message ACommand::buildMessage(IClient* sender, const std::string& target,
							   const std::string& text) const
{
	Message msg;
	msg.m_prefix = sender ? sender->getPrefix() : "";
	msg.m_command = getName();
	msg.m_params.push_back(target);
	msg.m_params.push_back(text);
	return msg;
}

void ACommand::broadcastToChannel(IChannel* channel, IClient* sender, const Message& message)
{
	if (!channel || !sender)
		return;

	std::string serialized = MessageParser::serialize(message);
	channel->broadcast(serialized, sender);
}

void ACommand::sendToClient(IClient* target, IClient* sender, const Message& message)
{
	if (!target || !sender)
		return;

	std::string serialized = MessageParser::serialize(message);
	target->getBuffer().appendWrite(serialized);
}
