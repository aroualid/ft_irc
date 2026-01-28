#pragma once

#include "core/IChannel.hpp"
#include "core/IClient.hpp"
#include "core/IServer.hpp"
#include "network/MessageBuffer.hpp"

class ClientState;

class Client : public IClient
{
private:
	int _fd;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	std::string _hostname;

	std::time_t m_lastActivity;
	std::time_t m_lastPingSent;

	enum ClientState
	{
		HANDSHAKE,
		LOGIN,
		REGISTERED,
		DISCONNECTED
	};

	ClientState _state;
	bool _passwordProvided;

	MessageBuffer _buffer;
	IServer& _server;
	std::set< IChannel* > _channels;

public:
	Client(int fd, const std::string& hostname, IServer& server);
	~Client();

	// Getters
	int getFd() const;
	const std::string& getNickname() const;
	const std::string& getUsername() const;
	const std::string& getRealname() const;
	const std::string& getHostname() const;
	IServer* getServer() const;

	bool isPasswordProvided() const;
	bool isRegistered() const;

	// State management
	void setNickname(const std::string& nick);
	void setUsername(const std::string& user);
	void setRealname(const std::string& real);
	void setPasswordProvided(bool provided);
	void attemptRegistration();

	// Channel membership
	void joinChannel(IChannel* channel);
	void leaveChannel(IChannel* channel);
	bool isInChannel(const std::string& channel) const;
	const std::set< IChannel* >& getChannels() const;
	// Buffer access
	IMessageBuffer& getBuffer();
	const IMessageBuffer& getBuffer() const;

	std::time_t getLastActivity() const;
	void updateLastActivity();
	std::time_t getLastPingSent() const;
	void setLastPingSent(std::time_t last_ping);

	std::string getPrefix() const; // "nick!user@host"
};
