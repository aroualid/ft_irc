#pragma once
#include <iostream>
#include <stdexcept>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/poll.h>

#include <netdb.h> 
#include <sstream>
#include <map>
#include <set>

//#include "PollSocketManager.hpp"

#include "core/IServer.hpp"
#include "IClient.hpp"
#include "IChannel.hpp"
#include "core/Config.hpp"
#include "network/ISocketManager.hpp"

class Server: public IServer
{
	private:
		const Config	m_cfg;
		int				m_listenFd;
		ISocketManager  *m_sm;

		std::map<int, IClient*> m_clients;
		std::map<std::string, IClient*> m_clientsByNick;
		std::map<std::string, IChannel*>m_channels;


		IClient	*getClient(int fd);
		void	onIrcLine(int fd, const std::string &line);

		void	handleDisconnections(int fd, unsigned int evt);
		void	acceptNewClients();
		void	readClientsData(int fd);
		void	disconnectClient(int fd);

		void	writeClientsData(int fd);
		
		;

	public:
		Server(const Config &cfg);
		~Server();
		void	run();

		//IServer
		int	getPort() const;
		const std::string &getPassword()const;

		IClient		*getClientByNickname(const std::string &nick);
		void		registerClient(const std::string &nick, IClient *client);
		void		unregisterClient(const std::string &nick);
		
		IChannel	*getChannel(const std::string  &name);
		IChannel	*createChannel(const std::string  &name, IClient *creator);
		void		deleteChannelIfEmpty(IChannel *channel);
		size_t		getChannelCount() const;
		std::string	getServerName()const;
		bool requiresPassword() const;
};
