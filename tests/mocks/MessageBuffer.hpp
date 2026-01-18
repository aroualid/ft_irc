#pragma once

#include "core/IMessageBuffer.hpp"
#include <string>

class MessageBuffer : public IMessageBuffer
{
private:
	std::string m_readBuffer;
	std::string m_writeBuffer;

public:
	MessageBuffer();
	virtual ~MessageBuffer();

	void appendRead(const std::string& data);
	void appendWrite(const std::string& data);
	const std::string& getWriteBuffer() const;
	void clearWriteBuffer();
};
