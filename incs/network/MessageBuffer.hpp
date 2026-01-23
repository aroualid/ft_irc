#pragma once

#include <vector>
#include <iostream>
#include "core/IMessageBuffer.hpp"

class MessageBuffer : public IMessageBuffer
{
	private:
		std::string m_readBuffer;
		std::string m_writeBuffer;

	public:
		MessageBuffer();
		
		//read
		void appendRead(const std::string& data);
		bool hasCompleteMessage() const;
		std::string getNextMessage();
		size_t	getReadBufferSize() const;

		//write
		void appendWrite(const std::string& data);
		void consumeWriteBuffer(size_t bytes);
		const std::string& getWriteBuffer() const;
		void clearWriteBuffer();
};