#pragma once

#include "core/IServer.hpp"
#include <map>
#include <string>

class Server : public IServer
{
private:
	int m_port;
	std::string m_password;
	std::map< std::string, IClient* > m_clients;
	std::map< std::string, IChannel* > m_channels;

public:
	Server(int port, const std::string& password);
	virtual ~Server();

	int getPort() const;
	const std::string& getPassword() const;

	IClient* getClientByNickname(const std::string& nick);
	void registerClient(const std::string& nick, IClient* client);
	void unregisterClient(const std::string& nick);

	IChannel* getChannel(const std::string& name);
	IChannel* createChannel(const std::string& name, IClient* creator);
	void deleteChannelIfEmpty(IChannel* channel);
	size_t getChannelCount() const;
};
