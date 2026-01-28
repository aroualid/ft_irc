#include "network/MessageBuffer.hpp"

MessageBuffer::MessageBuffer() : m_readBuffer(), m_writeBuffer()
{
}

void MessageBuffer::appendRead(const std::string& data)
{
	m_readBuffer += data;
}

bool MessageBuffer::hasCompleteMessage() const
{
	return (m_readBuffer.find("\r\n") != std::string::npos ||
			m_readBuffer.find('\n') != std::string::npos);
}

std::string MessageBuffer::getNextMessage()
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

size_t MessageBuffer::getReadBufferSize() const
{
	return m_readBuffer.size();
}

void MessageBuffer::appendWrite(const std::string& data)
{
	m_writeBuffer += data;
}

const std::string& MessageBuffer::getWriteBuffer() const
{
	return (m_writeBuffer);
}

void MessageBuffer::consumeWriteBuffer(size_t bytes)
{
	if (bytes >= m_writeBuffer.size())
		m_writeBuffer.clear();
	else
		m_writeBuffer.erase(0, bytes);
}

void MessageBuffer::clearWriteBuffer()
{
	m_writeBuffer.clear();
}
