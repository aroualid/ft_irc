#include "bot/BotMessageBuffer.hpp"

BotMessageBuffer::BotMessageBuffer() : m_readBuffer(), m_writeBuffer()
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
