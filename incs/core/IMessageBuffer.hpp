#pragma once

#include <string>

class IMessageBuffer
{
public:
	virtual ~IMessageBuffer()
	{
	}

	virtual void appendRead(const std::string& data) = 0;
	virtual void appendWrite(const std::string& data) = 0;
	virtual const std::string& getWriteBuffer() const = 0;
	virtual void clearWriteBuffer() = 0;
};
