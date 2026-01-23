#pragma once
#include <cstring>
#include <iostream>
#include <stdexcept>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <sys/poll.h>

#include <map>
#include <netdb.h>
#include <set>
#include <sstream>

// #include "PollSocketManager.hpp"

#include "IChannel.hpp"
#include "IClient.hpp"
#include "core/Config.hpp"
#include "core/IServer.hpp"
#include "network/ISocketManager.hpp"

class Server : public IServer
{
private:
	const Config m_cfg;
	int m_listenFd;
	ISocketManager* m_sm;

	std::map< int, IClient* > m_clients;
	std::map< std::string, IClient* > m_clientsByNick;
	std::map< std::string, IChannel* > m_channels;

	IClient* getClient(int fd);
	void onIrcLine(int fd, const std::string& line);

	void handleDisconnections(int fd, unsigned int evt);
	void acceptNewClients();
	void readClientsData(int fd);
	void disconnectClient(int fd);

	void writeClientsData(int fd);

	;

public:
	Server(const Config& cfg);
	~Server();
	void run();

	// IServer
	int getPort() const;
	const std::string& getPassword() const;

	IClient* getClientByNickname(const std::string& nick);
	void registerClient(const std::string& nick, IClient* client);
	void unregisterClient(const std::string& nick);

	IChannel* getChannel(const std::string& name);
	IChannel* createChannel(const std::string& name, IClient* creator);
	void deleteChannelIfEmpty(IChannel* channel);
	size_t getChannelCount() const;
	std::string getServerName() const;
	bool requiresPassword() const;
};
