#pragma once

#include "IChannel.hpp"
#include "IClient.hpp"
#include "IServer.hpp"
#include "bot/BotClient.hpp"
#include "bot/IBot.hpp"
#include <fstream>

class	NielBot : public IBot
{
	private:
		IServer& m_server;
		BotClient* m_client;

		void	sendToChannel(IChannel* channel, const std::string& msg);

	public:
		NielBot(IServer& server, const std::string& nick = "NielBot");
		~NielBot();

		void	onPrivateMessage(IClient* sender, const std::string& msg);
		void	onChannelMessage(IClient* sender, IChannel* channel, const std::string& msg);

		IClient* getClient();

		void	joinChannel(const std::string& channelName);
		void	loadAscii(const std::string& path);
};