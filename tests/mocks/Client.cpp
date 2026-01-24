#include "Client.hpp"
#include "IChannel.hpp"
#include <iostream>

ClientMock::ClientMock(int fd, const std::string& hostname, IServer& server )
	: m_fd(fd), m_nickname(""), m_username(""), m_realname(""), m_hostname(hostname),
	  m_passwordProvided(false), m_registered(false), m_server(&server)
{
	std::cout << "[MOCK] ClientMock created: fd=" << fd << " host=" << hostname << '\n';
}

ClientMock::~ClientMock()
{
	std::cout << "[MOCK] ClientMock destroyed: fd=" << m_fd << '\n';
}

int ClientMock::getFd() const
{
	return m_fd;
}
const std::string& ClientMock::getNickname() const
{
	return m_nickname;
}
const std::string& ClientMock::getUsername() const
{
	return m_username;
}
const std::string& ClientMock::getRealname() const
{
	return m_realname;
}
const std::string& ClientMock::getHostname() const
{
	return m_hostname;
}

bool ClientMock::isPasswordProvided() const
{
	return m_passwordProvided;
}
bool ClientMock::isRegistered() const
{
	return m_registered;
}

void ClientMock::setNickname(const std::string& nick)
{
	m_nickname = nick;
	std::cout << "[MOCK] Nickname set to: " << nick << '\n';
	attemptRegistration();
}

void ClientMock::setUsername(const std::string& user)
{
	m_username = user;
	std::cout << "[MOCK] Username set to: " << user << '\n';
	attemptRegistration();
}

void ClientMock::setRealname(const std::string& real)
{
	m_realname = real;
	std::cout << "[MOCK] Realname set to: " << real << '\n';
}

void ClientMock::setPasswordProvided(bool provided)
{
	m_passwordProvided = provided;
	std::cout << "[MOCK] Password provided: " << (provided ? "YES" : "NO") << '\n';
	attemptRegistration();
}

void ClientMock::attemptRegistration()
{
	bool wasRegistered = m_registered;
	m_registered = m_passwordProvided && !m_nickname.empty() && !m_username.empty();
	if (m_registered && !wasRegistered)
		std::cout << "[MOCK] ClientMock is now REGISTERED!" << '\n';
}

void ClientMock::joinChannel(IChannel* channel)
{
	m_channels.insert(channel);
	std::cout << "[MOCK] ClientMock joined channel: " << channel << '\n';
}

void ClientMock::leaveChannel(IChannel* channel)
{
	m_channels.erase(channel);
	std::cout << "[MOCK] ClientMock left channel: " << channel << '\n';
}

bool ClientMock::isInChannel(const std::string& channelName) const
{
	for (std::set< IChannel* >::const_iterator it = m_channels.begin(); it != m_channels.end();
		 ++it)
	{
		if ((*it)->getName() == channelName)
			return true;
	}
	return false;
}

const std::set< IChannel* >& ClientMock::getChannels() const
{
	return m_channels;
}

IMessageBuffer& ClientMock::getBuffer()
{
	return m_buffer;
}

const IMessageBuffer& ClientMock::getBuffer() const
{
	return m_buffer;
}

std::string ClientMock::getPrefix() const
{
	if (m_nickname.empty())
		return m_hostname;
	std::string prefix = m_nickname;
	if (!m_username.empty())
		prefix += "!" + m_username + "@" + m_hostname;
	return prefix;
}

IServer* ClientMock::getServer() const
{
	return m_server;
}
