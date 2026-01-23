#pragma once

#include "IChannel.hpp"
#include "core/IClient.hpp"
#include "mocks/MessageBuffer.hpp"
#include <set>
#include <string>

class ClientMock : public IClient
{
private:
	int m_fd;
	std::string m_nickname;
	std::string m_username;
	std::string m_realname;
	std::string m_hostname;
	bool m_passwordProvided;
	bool m_registered;
	MessageBufferMock m_buffer;
	std::set< IChannel * > m_channels;

public:
	ClientMock(int fd, const std::string& hostname);
	virtual ~ClientMock();

	int getFd() const;
	const std::string& getNickname() const;
	const std::string& getUsername() const;
	const std::string& getRealname() const;
	const std::string& getHostname() const;

	bool isPasswordProvided() const;
	bool isRegistered() const;

	void setNickname(const std::string& nick);
	void setUsername(const std::string& user);
	void setRealname(const std::string& real);
	void setPasswordProvided(bool provided);
	void attemptRegistration();

	void joinChannel(IChannel *channel);
	void leaveChannel(IChannel *channel);
	bool isInChannel(const std::string& channel) const;
	const std::set< IChannel * >& getChannels() const;

	IMessageBuffer& getBuffer();
	const IMessageBuffer& getBuffer() const;

	std::string getPrefix() const;
};
