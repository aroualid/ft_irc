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

#include "Config.hpp"
#include "EpollSocketManager.hpp"

class Server
{
	private:
		const Config	m_cfg;
		int				m_listenFd;
		ISocketManager  *m_sm;

		std::map<int, std::string> m_inbuf; // temp; later map<int, Client*>
		void	onIrcLine(int fd, const std::string &line);

		void	handleDisconnections(int fd, unsigned int evt);
		void	acceptNewClients();
		void	readClientsData(int fd);
		void	disconnectClient(int fd);

	public:
		Server(const Config &cfg);
		~Server();
		void	run();
};
