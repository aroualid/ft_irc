#include "bot/BotClient.hpp"
#include "IChannel.hpp"
#include "core/IServer.hpp"

int BotClient::s_nextBotId = -1;

BotClient::BotClient(const std::string& nick, IServer& server)
	: m_id(s_nextBotId--), m_nickname(nick), m_username(""), m_realname(""), m_hostname("internal"),
	  m_server(server), _passwordProvided(true), m_lastActivity(std::time(NULL)), m_lastPingSent(0)
{
}

BotClient::~BotClient()
{
}

IServer* BotClient::getServer() const
{
	return &m_server;
}

int BotClient::getFd() const
{
	return m_id;
}

const std::string& BotClient::getNickname() const
{
	return (m_nickname);
}

const std::string& BotClient::getUsername() const
{
	return (m_username);
}

const std::string& BotClient::getRealname() const
{
	return (m_realname);
}

const std::string& BotClient::getHostname() const
{
	return (m_hostname);
}

bool BotClient::isPasswordProvided() const
{
	return true;
}

bool BotClient::isRegistered() const
{
	return true;
}

void BotClient::setNickname(const std::string& nick)
{
	m_nickname = nick;
	attemptRegistration();
}

void BotClient::setUsername(const std::string& user)
{
	m_username = user;
	attemptRegistration();
}

void BotClient::setRealname(const std::string& real)
{
	m_realname = real;
}

void BotClient::setPasswordProvided(bool provided)
{
	_passwordProvided = provided;
	attemptRegistration();
}

void BotClient::joinChannel(IChannel* channel)
{
	m_channels.insert(channel);
}

void BotClient::leaveChannel(IChannel* channel)
{
	m_channels.erase(channel);
}

bool BotClient::isInChannel(const std::string& channelName) const
{
	for (std::set< IChannel* >::const_iterator it = m_channels.begin(); it != m_channels.end();
		 ++it)
	{
		if ((*it)->getName() == channelName)
			return true;
	}
	return false;
}

const std::set< IChannel* >& BotClient::getChannels() const
{
	return m_channels;
}

void BotClient::attemptRegistration()
{
	if (m_nickname.empty())
		return;
	if (m_username.empty())
		return;
	if (m_server.requiresPassword() && !_passwordProvided)
		return;
	m_server.registerClient(m_nickname, this);
}

IMessageBuffer& BotClient::getBuffer()
{
	return m_buffer;
}

const IMessageBuffer& BotClient::getBuffer() const
{
	return m_buffer;
}

std::time_t BotClient::getLastActivity() const
{
	return m_lastActivity;
}

void BotClient::updateLastActivity()
{
	m_lastActivity = std::time(NULL);
	m_lastPingSent = 0;
}

std::time_t BotClient::getLastPingSent() const
{
	return m_lastPingSent;
}

void BotClient::setLastPingSent(std::time_t last_ping)
{
	m_lastPingSent = last_ping;
}

std::string BotClient::getPrefix() const
{
	if (m_nickname.empty())
		return m_hostname;

	std::string prefix = m_nickname;
	if (!m_username.empty())
		prefix += "!" + m_username + "@" + m_hostname;
	return prefix;
}

void BotClient::setBot(IBot* bot)
{
	(void)bot;
}
