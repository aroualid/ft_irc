#pragma once

#include "IChannel.hpp"
#include "IClient.hpp"
#include "IServer.hpp"
#include "bot/IBot.hpp"

class BotClient;

class MiaouBot : public IBot
{
private:
	IServer& m_server;
	BotClient* m_client;

	void sendToChannel(IChannel* channel, const std::string& message);

public:
	MiaouBot(IServer& server, const std::string& nick = "Larry");
	~MiaouBot();

	void onPrivateMessage(IClient* sender, const std::string& message);
	void onChannelMessage(IClient* sender, IChannel* channel, const std::string& message);

	IClient* getClient();

	void joinChannel(const std::string& channelName);
};
