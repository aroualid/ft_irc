#pragma once

#include "BotMessageBuffer.hpp"
#include "IBot.hpp"
#include "IChannel.hpp"
#include "IClient.hpp"
#include "IServer.hpp"
#include "bot/BotMessageBuffer.hpp"

class BotClient : public IClient
{
private:
	static int s_nextBotId; // Starts at -1, decrements
	int m_id;				// Negative FD
	std::string m_nickname;
	std::string m_username;
	std::string m_realname;
	std::string m_hostname;
	IServer& m_server;
	std::set< IChannel* > m_channels;
	BotMessageBuffer m_buffer;
	bool _passwordProvided;

	std::time_t m_lastActivity;
	std::time_t m_lastPingSent;

public:
	BotClient(const std::string& nick, IServer& server);
	~BotClient();

	int getFd() const; // Returns negative m_id
	const std::string& getNickname() const;
	const std::string& getUsername() const;
	const std::string& getRealname() const;
	const std::string& getHostname() const; // Returns "internal"
	IServer* getServer() const;
	std::string getPrefix() const; // "nick!bot@internal"

	bool isRegistered() const;		 // Always returns true
	bool isPasswordProvided() const; // Always returns true

	void setNickname(const std::string& nick);
	void setUsername(const std::string& user);
	void setRealname(const std::string& real);
	void setPasswordProvided(bool provided);
	void attemptRegistration();

	void joinChannel(IChannel* channel);
	void leaveChannel(IChannel* channel);
	bool isInChannel(const std::string& channel) const;
	const std::set< IChannel* >& getChannels() const;
	// Buffer access

	std::time_t getLastActivity() const;
	void updateLastActivity();
	std::time_t getLastPingSent() const;
	void setLastPingSent(std::time_t last_ping);

	IMessageBuffer& getBuffer();
	const IMessageBuffer& getBuffer() const;
	void setBot(IBot* bot); // Links to bot handler
};
