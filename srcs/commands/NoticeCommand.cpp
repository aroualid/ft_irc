#include "commands/NoticeCommand.hpp"
#include "core/IChannel.hpp"
#include "core/IMessageBuffer.hpp"
#include "protocol/MessageParser.hpp"

NoticeCommand::NoticeCommand(IServer& server) : ACommand(server)
{
}

NoticeCommand::~NoticeCommand()
{
}

std::vector< std::string > NoticeCommand::splitByComma(const std::string& str) const
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

void NoticeCommand::sendToTarget(IClient* client, const std::string& target,
								 const std::string& text)
{
	if (!target.empty() && (target[0] == '#' || target[0] == '&'))
	{
		IChannel* channel = m_server.getChannel(target);
		if (!channel)
			return;

		if (!channel->hasMember(client))
			return;

		Message notice;
		notice.m_prefix = client->getPrefix();
		notice.m_command = getName();
		notice.m_params.push_back(target);
		notice.m_params.push_back(text);

		std::string serialized = MessageParser::serialize(notice);
		channel->broadcast(serialized, client);
	}
	else
	{
		IClient* targetClient = m_server.getClientByNickname(target);
		if (!targetClient)
			return;

		Message notice;
		notice.m_prefix = client->getPrefix();
		notice.m_command = getName();
		notice.m_params.push_back(target);
		notice.m_params.push_back(text);

		std::string serialized = MessageParser::serialize(notice);
		targetClient->getBuffer().appendWrite(serialized);
	}
}

void NoticeCommand::execute(IClient* client, const Message& message)
{
	if (message.m_params.size() < minParams())
		return;

	const std::string& targets = message.m_params[0];
	const std::string& text = message.m_params[1];

	std::vector< std::string > targetList = splitByComma(targets);

	for (std::size_t i = 0; i < targetList.size(); i++)
	{
		if (!targetList[i].empty())
			sendToTarget(client, targetList[i], text);
	}
}
