#include "Client.hpp"
#include <iostream>

Client::Client(int fd, const std::string& hostname)
	: m_fd(fd), m_nickname(""), m_username(""), m_realname(""), m_hostname(hostname),
	  m_passwordProvided(false), m_registered(false), m_buffer(), m_channels()
{
	std::cout << "[MOCK] Client created: fd=" << fd << " host=" << hostname << std::endl;
}

Client::~Client()
{
	std::cout << "[MOCK] Client destroyed: fd=" << m_fd << std::endl;
}

int Client::getFd() const
{
	return m_fd;
}
const std::string& Client::getNickname() const
{
	return m_nickname;
}
const std::string& Client::getUsername() const
{
	return m_username;
}
const std::string& Client::getRealname() const
{
	return m_realname;
}
const std::string& Client::getHostname() const
{
	return m_hostname;
}

bool Client::isPasswordProvided() const
{
	return m_passwordProvided;
}
bool Client::isRegistered() const
{
	return m_registered;
}
bool Client::isAuthenticated() const
{
	return m_passwordProvided;
}

void Client::setNickname(const std::string& nick)
{
	m_nickname = nick;
	std::cout << "[MOCK] Nickname set to: " << nick << std::endl;
	updateRegistrationState();
}

void Client::setUsername(const std::string& user)
{
	m_username = user;
	std::cout << "[MOCK] Username set to: " << user << std::endl;
	updateRegistrationState();
}

void Client::setRealname(const std::string& real)
{
	m_realname = real;
	std::cout << "[MOCK] Realname set to: " << real << std::endl;
}

void Client::setPasswordProvided(bool provided)
{
	m_passwordProvided = provided;
	std::cout << "[MOCK] Password provided: " << (provided ? "YES" : "NO") << std::endl;
	updateRegistrationState();
}

void Client::updateRegistrationState()
{
	bool wasRegistered = m_registered;
	m_registered = m_passwordProvided && !m_nickname.empty() && !m_username.empty();
	if (m_registered && !wasRegistered)
		std::cout << "[MOCK] Client is now REGISTERED!" << std::endl;
}

void Client::joinChannel(const std::string& channel)
{
	m_channels.insert(channel);
	std::cout << "[MOCK] Client joined channel: " << channel << std::endl;
}

void Client::leaveChannel(const std::string& channel)
{
	m_channels.erase(channel);
	std::cout << "[MOCK] Client left channel: " << channel << std::endl;
}

bool Client::isInChannel(const std::string& channel) const
{
	return m_channels.find(channel) != m_channels.end();
}

const std::set< std::string >& Client::getChannels() const
{
	return m_channels;
}

IMessageBuffer& Client::getBuffer()
{
	return m_buffer;
}

const IMessageBuffer& Client::getBuffer() const
{
	return m_buffer;
}

std::string Client::getPrefix() const
{
	if (m_nickname.empty())
		return m_hostname;
	std::string prefix = m_nickname;
	if (!m_username.empty())
		prefix += "!" + m_username + "@" + m_hostname;
	return prefix;
}
