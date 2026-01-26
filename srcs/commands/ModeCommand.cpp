#include "commands/ModeCommand.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "protocol/MessageParser.hpp"
#include <new>

REGISTER_COMMAND(ModeCommand, irc::MODE, "MODE");

ModeCommand::ModeCommand(IServer& server) : ACommand(server)
{
}

ModeCommand::~ModeCommand()
{
}

bool ModeCommand::isSupportedMode(char mode)
{
	return mode == 'i' || mode == 't' || mode == 'k' || mode == 'o' || mode == 'l';
}

bool ModeCommand::modeRequiresParamOnSet(char mode)
{
	return mode == 'k' || mode == 'o' || mode == 'l';
}

bool ModeCommand::modeRequiresParamOnUnset(char mode)
{
	return mode == 'o';
}

void ModeCommand::doExecute(IClient* client, const Message& message)
{
	const std::string& target = message.m_params[0];

	if (target[0] != '#' && target[0] != '&')
		return;

	const std::string& channelName = target;
	IChannel* channel = m_server.getChannel(channelName);
	if (!channel)
	{
		sendReply(client, NumericReply::noSuchChannel(client->getNickname(), channelName));
		return;
	}

	if (message.m_params.size() == 1)
	{
		std::string modes = channel->getModeString();
		sendReply(client, NumericReply::channelModeIs(client->getNickname(), channelName, modes));
		return;
	}

	const std::string& modeString = message.m_params[1];

	bool hasSupportedMode = false;
	for (size_t i = 0; i < modeString.size(); ++i)
	{
		char c = modeString[i];
		if (c != '+' && c != '-' && isSupportedMode(c))
		{
			hasSupportedMode = true;
			break;
		}
	}

	if (!hasSupportedMode)
	{
		for (size_t i = 0; i < modeString.size(); ++i)
		{
			char c = modeString[i];
			if (c != '+' && c != '-')
				sendReply(client, NumericReply::unknownMode(client->getNickname(), c));
		}
		return;
	}

	if (!channel->hasMember(client))
	{
		sendReply(client, NumericReply::notOnChannel(client->getNickname(), channelName));
		return;
	}

	if (!channel->isOperator(client))
	{
		sendReply(client, NumericReply::chanOpPrivsNeeded(client->getNickname(), channelName));
		return;
	}
	size_t paramIndex = 2;

	bool isSet = true;
	std::string appliedPlus;
	std::string appliedMinus;
	std::vector< std::string > appliedParams;

	for (size_t i = 0; i < modeString.size(); ++i)
	{
		char c = modeString[i];

		if (c == '+')
		{
			isSet = true;
			continue;
		}
		if (c == '-')
		{
			isSet = false;
			continue;
		}

		if (c != 'i' && c != 't' && c != 'k' && c != 'o' && c != 'l')
		{
			sendReply(client, NumericReply::unknownMode(client->getNickname(), c));
			continue;
		}

		std::string param;
		bool needsParam = isSet ? modeRequiresParamOnSet(c) : modeRequiresParamOnUnset(c);

		if (needsParam)
		{
			if (paramIndex < message.m_params.size())
			{
				param = message.m_params[paramIndex];
				++paramIndex;
			}
			else
			{
				continue;
			}
		}
		if (channel->applyMode(c, isSet, param, client))
		{
			if (isSet)
				appliedPlus += c;
			else
				appliedMinus += c;
			if (!param.empty())
				appliedParams.push_back(param);
		}
	}

	if (appliedPlus.empty() && appliedMinus.empty())
		return;

	std::string finalModeString;
	if (!appliedPlus.empty())
		finalModeString += "+" + appliedPlus;
	if (!appliedMinus.empty())
		finalModeString += "-" + appliedMinus;

	Message modeMsg;
	modeMsg.m_prefix = client->getPrefix();
	modeMsg.m_command = "MODE";
	modeMsg.m_params.push_back(channelName);
	modeMsg.m_params.push_back(finalModeString);

	for (size_t i = 0; i < appliedParams.size(); ++i)
		modeMsg.m_params.push_back(appliedParams[i]);

	channel->broadcast(MessageParser::serialize(modeMsg));
}

ACommand* ModeCommand::create(IServer& server)
{
	return new (std::nothrow) ModeCommand(server);
}
