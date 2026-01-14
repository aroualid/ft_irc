#include "Server.hpp"

static void	setNonBlocking(int fd)
{
	int	flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Function fcntl with command F_GETFL failed\n");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Function fcntl with command F_SETFL failed\n");
}

void	Server::disconnectClient(int fd)
{
	try{
		m_sm->removeSocket(fd);
	} catch (...){

	}
	close(fd);
	m_clients.erase(fd);
	std::cout << "Disconnected : " << fd << "\n";
}

void	Server::readClientsData(int fd)
{
	//std::cout << "readClientsData loop ready to call\n" << fd ;
	char	buffer[4096];

	while (true)
	{
		ssize_t	receiving = recv(fd, buffer, sizeof(buffer), 0);
		if (receiving > 0)
		{
			std::cout << fd << " sent " << receiving << " bytes of data: ";
			std::cout.write(buffer, receiving);
			std::cout << "\n";
		}
		else if (receiving == 0)
		{
			disconnectClient(fd);
			return ;
		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ;
			disconnectClient(fd);
			return ;
		}
	}
}

void	Server::acceptNewClients()
{
	while (true)
	{
		int	clientFd = accept(m_listenFd, 0, 0);
		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			throw std::runtime_error("Acceptation of a New Client failed\n");
		}
		setNonBlocking(clientFd);
		m_sm->addSocket(clientFd, EPOLLIN);

		m_clients.insert(clientFd);
		std::cout << "Client Accepted : " << clientFd << "\n";
	}
}

void	Server::handleDisconnections(int fd, unsigned int evt)
{
	std::cout << "handleDisconenctions loop ready to call\n" << fd << evt;
	if (fd == m_listenFd)
		throw std::runtime_error("Listening Socket disconnected\n");
	disconnectClient(fd);
}

static int	createListeningSocket(int port) //fd that listens to all interfaces trying to bind
{
	std::stringstream ss;
	ss << port;

	std::string portStr = ss.str();
	struct addrinfo		hints;
	struct addrinfo		*res = 0;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; //indicates that getaddrinfo() should return socket addresses for any address family (either IPv4 or IPv6, for example)
	hints.ai_socktype = SOCK_STREAM; //tcp 
	hints.ai_flags = AI_PASSIVE; // for all interfaces to bind to server (host == NULL)

	int	err = getaddrinfo(0, portStr.c_str(), &hints, &res);
	if (err != 0)
		throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(err));

	int	listenFd = -1; //listening fd not created yet
	for (struct addrinfo *p = res; p != 0; p = p->ai_next)
	{
		listenFd= socket(p->ai_family, p->ai_socktype, p->ai_protocol); //creating the listening socket
		if (listenFd == -1)
			continue;
		int	yes = 1;
		setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)); //
		if (bind(listenFd, p->ai_addr, p->ai_addrlen) == 0){
			if (listen(listenFd, 128) == 0)
			{
				freeaddrinfo(res);
				return(listenFd);
			}
		}
		close(listenFd); //useful ?
		listenFd = -1;

		}
		freeaddrinfo(res);
		throw std::runtime_error("Failed to bind/listen on port : " + portStr);
}


void	Server::run()
{
	std::cout << "Ready to run !\n";

	m_listenFd = createListeningSocket(m_cfg.getPort()); //the "door" of the server irc
	std::cout << "Listening...\n";

	m_sm = new EpollSocketManager(); //THE Manager of the Sockets

	setNonBlocking(m_listenFd);

	m_sm->addSocket(m_listenFd, EPOLLIN); // pending incoming connexions : 

	while (true)
	{
		int	n = m_sm->wait(-1); //waiting for incoming connexxions
		const std::vector<epoll_event> &evts = m_sm->getEvents(); //vector getting events

		for (int i = 0; i < n; i++)
		{
			int				fd = evts[i].data.fd;
			unsigned int	evt = evts[i].events;

			if (evt & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
			{
				handleDisconnections(fd, evt);
				continue;
			}
			if (fd == m_listenFd && (evt & EPOLLIN)) //new connexions
			{
				acceptNewClients();
			}
			else if (evt & EPOLLIN) //clients sent data
			{
				readClientsData(fd);
			}
		}
	}
}


Server::Server(const Config &cfg) : m_cfg(cfg), m_listenFd(-1), m_sm(0) {}

Server::~Server(){
	if (m_listenFd != -1)
		close(m_listenFd);
	delete(m_sm);
}