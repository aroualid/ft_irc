#pragma once

#include "MessageBuffer.hpp"
#include "core/IClient.hpp"
#include <set>
#include <string>

class Client : public IClient
{
private:
	int m_fd;
	std::string m_nickname;
	std::string m_username;
	std::string m_realname;
	std::string m_hostname;
	bool m_passwordProvided;
	bool m_registered;
	MessageBuffer m_buffer;
	std::set< std::string > m_channels;

public:
	Client(int fd, const std::string& hostname);
	virtual ~Client();

	int getFd() const;
	const std::string& getNickname() const;
	const std::string& getUsername() const;
	const std::string& getRealname() const;
	const std::string& getHostname() const;

	bool isPasswordProvided() const;
	bool isRegistered() const;
	bool isAuthenticated() const;

	void setNickname(const std::string& nick);
	void setUsername(const std::string& user);
	void setRealname(const std::string& real);
	void setPasswordProvided(bool provided);
	void updateRegistrationState();

	void joinChannel(const std::string& channel);
	void leaveChannel(const std::string& channel);
	bool isInChannel(const std::string& channel) const;
	const std::set< std::string >& getChannels() const;

	IMessageBuffer& getBuffer();
	const IMessageBuffer& getBuffer() const;

	std::string getPrefix() const;
};
