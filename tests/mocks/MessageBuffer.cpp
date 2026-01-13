#include "MessageBuffer.hpp"
#include <iostream>

MessageBuffer::MessageBuffer() {}
MessageBuffer::~MessageBuffer() {}

void MessageBuffer::appendRead(const std::string& data)
{
	m_readBuffer += data;
}

void MessageBuffer::appendWrite(const std::string& data)
{
	m_writeBuffer += data;
	std::cout << "[MOCK] Sent to client: " << data;
}

const std::string& MessageBuffer::getWriteBuffer() const
{
	std::cout << "[MOCK] Getting write buffer: " << m_writeBuffer;
	return m_writeBuffer;
}

void MessageBuffer::clearWriteBuffer()
{
	m_writeBuffer.clear();
}
