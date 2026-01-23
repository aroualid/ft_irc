#pragma once

#include <string>

class IMessageBuffer
{
public:
	virtual ~IMessageBuffer()
	{
	}

	//read
	virtual void		appendRead(const std::string& data) = 0;
	virtual bool 		hasCompleteMessage() const = 0;
	virtual std::string	getNextMessage() = 0;
	virtual size_t		getReadBufferSize() const = 0;




	//write
	virtual void appendWrite(const std::string& data) = 0;
	virtual void consumeWriteBuffer(size_t bytes) = 0;
	virtual const std::string& getWriteBuffer() const = 0;
	virtual void clearWriteBuffer() = 0;

};
