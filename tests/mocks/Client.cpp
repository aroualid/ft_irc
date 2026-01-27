#include "Client.hpp"
#include "IChannel.hpp"
#include "MockOutput.hpp"

ClientMock::ClientMock(int fd, const std::string& hostname, IServer& server)
	: m_fd(fd), m_nickname(""), m_username(""), m_realname(""), m_hostname(hostname),
	  m_passwordProvided(false), m_registered(false), m_lastActivity(std::time(NULL)),
	  m_lastPingSent(0), m_server(&server)
{
	MOCK_LOG("ClientMock created: fd=" << fd << " host=" << hostname);
}

ClientMock::~ClientMock()
{
	MOCK_LOG("ClientMock destroyed: fd=" << m_fd);
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
	MOCK_LOG("Nickname set to: " << nick);
	attemptRegistration();
}

void ClientMock::setUsername(const std::string& user)
{
	m_username = user;
	MOCK_LOG("Username set to: " << user);
	attemptRegistration();
}

void ClientMock::setRealname(const std::string& real)
{
	m_realname = real;
	MOCK_LOG("Realname set to: " << real);
}

void ClientMock::setPasswordProvided(bool provided)
{
	m_passwordProvided = provided;
	MOCK_LOG("Password provided: " << (provided ? "YES" : "NO"));
	attemptRegistration();
}

void ClientMock::attemptRegistration()
{
	bool wasRegistered = m_registered;
	m_registered = m_passwordProvided && !m_nickname.empty() && !m_username.empty();
	if (m_registered && !wasRegistered)
		MOCK_LOG("ClientMock is now REGISTERED!");
}

void ClientMock::joinChannel(IChannel* channel)
{
	m_channels.insert(channel);
	MOCK_LOG("ClientMock joined channel: " << channel->getName());
}

void ClientMock::leaveChannel(IChannel* channel)
{
	m_channels.erase(channel);
	MOCK_LOG("ClientMock left channel: " << channel->getName());
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

std::time_t ClientMock::getLastActivity() const
{
	return m_lastActivity;
}

void ClientMock::updateLastActivity()
{
	m_lastActivity = std::time(NULL);
	m_lastPingSent = 0;
}

std::time_t ClientMock::getLastPingSent() const
{
	return m_lastPingSent;
}

void ClientMock::setLastPingSent(std::time_t last_ping)
{
	m_lastPingSent = last_ping;
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
