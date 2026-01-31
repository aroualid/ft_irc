#pragma once
#include <iostream>
#include <cstddef>
#include <vector>

#include "core/IMessageBuffer.hpp"
#include "core/IServer.hpp"
#include "bot/IBot.hpp"
#include "protocol/Message.hpp"

class	BotMessageBuffer : public IMessageBuffer
{
	private:
		IServer	&m_server;
		IBot	*m_bot;
		std::string m_readBuffer;
		std::string m_writeBuffer;

		void	processIncomingMessage(const std::string& raw);
		void	parseAndDispatch(const Message &message);

	public:
		BotMessageBuffer(IServer& server);
		virtual	~BotMessageBuffer();

		void appendRead(const std::string& data);
		bool hasCompleteMessage() const;
		std::string getNextMessage();
		size_t getReadBufferSize() const;

		void appendWrite(const std::string& data);
		void consumeWriteBuffer(size_t bytes);
		const std::string& getWriteBuffer() const;
		void clearWriteBuffer();

		void	setBot(IBot* bot);
};
