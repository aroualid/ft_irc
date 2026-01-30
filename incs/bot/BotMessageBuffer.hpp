#pragma once
#include <iostream>
#include <cstddef>

#include "core/IMessageBuffer.hpp"

class	BotMessageBuffer : public IMessageBuffer
{
	private:
		std::string m_readBuffer;
		std::string m_writeBuffer;
	public:
		BotMessageBuffer();
		virtual	~BotMessageBuffer();

		void appendRead(const std::string& data);
		bool hasCompleteMessage() const;
		std::string getNextMessage();
		size_t getReadBufferSize() const;

	// write
		void appendWrite(const std::string& data);
		void consumeWriteBuffer(size_t bytes);
		const std::string& getWriteBuffer() const;
		void clearWriteBuffer();
};