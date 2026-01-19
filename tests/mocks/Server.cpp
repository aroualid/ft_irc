#include "Server.hpp"
#include "Channel.hpp"
#include <iostream>

Server::Server(int port, const std::string& password)
	: m_port(port), m_password(password), m_clients(), m_channels()
{
	std::cout << "[MOCK] Server created: port=" << port << " password=" << password << std::endl;
}

Server::~Server()
{
	for (std::map< std::string, IChannel* >::iterator it = m_channels.begin();
		 it != m_channels.end(); ++it)
		delete it->second;
	std::cout << "[MOCK] Server destroyed" << std::endl;
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
	std::map< std::string, IClient* >::iterator it = m_clients.find(nick);
	if (it != m_clients.end())
		return it->second;
	return NULL;
}

void Server::registerClient(const std::string& nick, IClient* client)
{
	m_clients[nick] = client;
	std::cout << "[MOCK] Server registered client: " << nick << std::endl;
}

void Server::unregisterClient(const std::string& nick)
{
	m_clients.erase(nick);
	std::cout << "[MOCK] Server unregistered client: " << nick << std::endl;
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
	std::cout << "[MOCK] Server created channel: " << name << std::endl;
	return channel;
}

void Server::deleteChannelIfEmpty(IChannel* channel)
{
	if (!channel || !channel->isEmpty())
		return;
	std::string name = channel->getName();
	m_channels.erase(name);
	delete channel;
	std::cout << "[MOCK] Server deleted empty channel: " << name << std::endl;
}

size_t Server::getChannelCount() const
{
	return m_channels.size();
}
