#include "Server.hpp"
#include "Channel.hpp"
#include "MockOutput.hpp"
#include "protocol/IrcUtils.hpp"

Server::Server(int port, const std::string& password)
	: m_port(port), m_password(password), m_clients(), m_channels()
{
	MOCK_LOG("Server created: port=" << port << " password=" << password);
}

Server::~Server()
{
	for (std::map< std::string, IChannel* >::iterator it = m_channels.begin();
		 it != m_channels.end(); ++it)
		delete it->second;
	MOCK_LOG("Server destroyed");
}

bool Server::requiresPassword() const
{
	return (!m_password.empty());
}

int Server::getPort() const
{
	return m_port;
}

const std::string& Server::getPassword() const
{
	return m_password;
}

IClient* Server::getClientByNickname(const std::string& nick)
{
	// RFC 1459: nickname comparison is case-insensitive
	for (std::map< std::string, IClient* >::iterator it = m_clients.begin(); it != m_clients.end();
		 ++it)
	{
		if (IrcUtils::iequals(it->first, nick))
			return it->second;
	}
	return NULL;
}

void Server::registerClient(const std::string& nick, IClient* client)
{
	m_clients[nick] = client;
	MOCK_LOG("Server registered client: " << nick);
}

void Server::unregisterClient(const std::string& nick)
{
	m_clients.erase(nick);
	MOCK_LOG("Server unregistered client: " << nick);
}

IChannel* Server::getChannel(const std::string& name)
{
	std::map< std::string, IChannel* >::iterator it = m_channels.find(name);
	if (it != m_channels.end())
		return it->second;
	return NULL;
}

IChannel* Server::createChannel(const std::string& name, IClient* creator)
{
	if (getChannel(name))
		return NULL;
	ChannelMock* channel = new ChannelMock(name);
	m_channels[name] = channel;
	if (creator)
		channel->addMember(creator);
	MOCK_LOG("Server created channel: " << name);
	return channel;
}

void Server::deleteChannelIfEmpty(IChannel* channel)
{
	if (!channel || !channel->isEmpty())
		return;
	std::string name = channel->getName();
	m_channels.erase(name);
	delete channel;
	MOCK_LOG("Server deleted empty channel: " << name);
}

size_t Server::getChannelCount() const
{
	return m_channels.size();
}

std::string Server::getServerName() const
{
	return "mock_server.serv";
}