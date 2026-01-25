#include "MessageBuffer.hpp"

MessageBufferMock::MessageBufferMock() : m_readBuffer(), m_writeBuffer()
{
}

MessageBufferMock::~MessageBufferMock()
{
}

void MessageBufferMock::appendRead(const std::string& data)
{
	m_readBuffer += data;
}

bool MessageBufferMock::hasCompleteMessage() const
{
	return (m_readBuffer.find("\r\n") != std::string::npos);
}

std::string MessageBufferMock::getNextMessage()
{
	std::string::size_type pos = m_readBuffer.find("\r\n");
	if (pos == std::string::npos)
		return ("");
	std::string line = m_readBuffer.substr(0, pos);
	m_readBuffer.erase(0, pos + 2);
	return (line);
}

size_t MessageBufferMock::getReadBufferSize() const
{
	return m_readBuffer.size();
}

void MessageBufferMock::appendWrite(const std::string& data)
{
	m_writeBuffer += data;
}

const std::string& MessageBufferMock::getWriteBuffer() const
{
	return (m_writeBuffer);
}

void MessageBufferMock::consumeWriteBuffer(size_t bytes)
{
	if (bytes >= m_writeBuffer.size())
		m_writeBuffer.clear();
	else
		m_writeBuffer.erase(0, bytes);
}

void MessageBufferMock::clearWriteBuffer()
{
	m_writeBuffer.clear();
}
