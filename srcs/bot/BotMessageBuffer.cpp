#include "bot/BotMessageBuffer.hpp"
#include "CommandType.hpp"
#include "IClient.hpp"
#include "Logger.hpp"
#include "protocol/Message.hpp"
#include "protocol/MessageParser.hpp"

BotMessageBuffer::BotMessageBuffer(IServer& server) : m_server(server), m_bot(NULL) 
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
	processIncomingMessage(data);
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

void BotMessageBuffer::setBot(IBot *bot)
{
	m_bot = bot;
}

void	BotMessageBuffer::parseAndDispatch(const Message &message)
{

	if (message.m_command_type != irc::PRIVMSG)
		return;

	std::string	target = message.m_params[0];
	std::string	text = message.m_params[1];
	std::string	senderNick = message.m_prefix.substr(0, message.m_prefix.find('!'));

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
	Message receivedMsg = MessageParser::parse(raw);
	if (!receivedMsg.isValid())
		return ;

	parseAndDispatch(receivedMsg);
}

//:prefix COMMAND param1 param2 :trailing parameter with spaces
