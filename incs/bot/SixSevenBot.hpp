#pragma once

#include "IChannel.hpp"
#include "IClient.hpp"
#include "IServer.hpp"
#include "bot/IBot.hpp"

class BotClient;

class SixSevenBot : public IBot
{
private:
	IServer& m_server;
	BotClient* client;

	void sendToChannel(IChannel* channel, const std::string& message);

public:
	SixSevenBot(IServer& server, const std::string& nick = "The 67 Kid");
	~SixSevenBot();

	void onPrivateMessage(IClient* sender, const std::string& message);
	void onChannelMessage(IClient* sender, IChannel* channel, const std::string& message);

	IClient* getClient();

	void joinChannel(const std::string& channelName);
};
