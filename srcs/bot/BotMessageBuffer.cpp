#include "bot/BotMessageBuffer.hpp"

BotMessageBuffer::BotMessageBuffer(IServer& server) : m_server(server), m_readBuffer(), m_writeBuffer()
{
}

BotMessageBuffer::~BotMessageBuffer(){}

void BotMessageBuffer::appendRead(const std::string& data)
{
	m_readBuffer += data;
}

bool BotMessageBuffer::hasCompleteMessage() const
{
	return (m_readBuffer.find("\r\n") != std::string::npos ||
			m_readBuffer.find('\n') != std::string::npos);
}

std::string BotMessageBuffer::getNextMessage()
{
	std::string::size_type pos = m_readBuffer.find("\r\n");
	if (pos != std::string::npos)
	{
		std::string line = m_readBuffer.substr(0, pos);
		m_readBuffer.erase(0, pos + 2);
		return (line);
	}
	pos = m_readBuffer.find('\n');
	if (pos != std::string::npos)
	{
		std::string line = m_readBuffer.substr(0, pos);
		m_readBuffer.erase(0, pos + 1);
		return (line);
	}
	return ("");
}

size_t BotMessageBuffer::getReadBufferSize() const
{
	return m_readBuffer.size();
}

void BotMessageBuffer::appendWrite(const std::string& data)
{
	m_writeBuffer += data;
}

const std::string& BotMessageBuffer::getWriteBuffer() const
{
	return (m_writeBuffer);
}

void BotMessageBuffer::consumeWriteBuffer(size_t bytes)
{
	if (bytes >= m_writeBuffer.size())
		m_writeBuffer.clear();
	else
		m_writeBuffer.erase(0, bytes);
}

void BotMessageBuffer::clearWriteBuffer()
{
	m_writeBuffer.clear();
}


void	BotMessageBuffer::parseAndDispatch(const std::string& prefix, const std::string& command, const std::vector<std::string>& params)
{
	if (command != "PRIVMSG" || params.size() < 2)
		return;

	std::string	target = params[0];
	std::string	text = params[1];

	std::string	senderNick = prefix.substr(0, prefix.find('!'));
	IClient* sender = m_server.getClientByNickname(senderNick);
	if (!sender)
		return;

	if (target[0] == '#')
	{
		IChannel* channel = m_server.getChannel(target);
		if (channel)
			m_bot->onChannelMessage(sender, channel, text);
	}
	else
	{
		m_bot->onPrivateMessage(sender, text);
	}
}

void	BotMessageBuffer::processIncomingMessage(const std::string& raw)
{
	std::string line = raw;
	if (line.size() >= 2 && line.substr(line.size()-2) == "\r\n")
		line = line.substr(0, line.size()-2);
	//\r\n removed

	std::string	prefix;
	std::string	command;
	std::vector<std::string> params;

	size_t	pos = 0;
	if (line[0] == ':')
	{
		pos = line.find(' ');
		prefix = line.substr(1, pos - 1);
		pos++;
	}
	//we've got the prefix
	
	size_t	nextSpace = line.find(' ', pos);
	command = line.substr(pos, nextSpace - pos);
	//we've got the command

	parseAndDispatch(prefix, command, params);
}

//:prefix COMMAND param1 param2 :trailing parameter with spaces