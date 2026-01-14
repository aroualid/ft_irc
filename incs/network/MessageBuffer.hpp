#pragma once

#include <vector>
#include <iostream>

class MessageBuffer {
private:
	std::string m_readBuffer;
	std::string m_writeBuffer;

public:
	MessageBuffer();
	void appendRead(const std::string& data);
	bool hasCompleteMessage() const;
	std::string getNextMessage();
	size_t	getReadBufferSize() const;

	void appendWrite(const std::string& data);
	const std::string& getWriteBuffer() const;
	void consumeWriteBuffer(size_t bytes);
};