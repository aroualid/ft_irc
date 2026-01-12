#include "Server.hpp"

// static void acceptAndPrintClient(int listenFd)
// {
//     struct sockaddr_storage clientAddr;
//     socklen_t len = sizeof(clientAddr);

//     int clientFd = accept(listenFd, (struct sockaddr*)&clientAddr, &len);
//     if (clientFd == -1)
//         return; // in non-blocking mode, this can happen (EAGAIN)

//     char ipstr[INET6_ADDRSTRLEN];
//     int clientPort = 0;

//     if (clientAddr.ss_family == AF_INET) {
//         struct sockaddr_in* s = (struct sockaddr_in*)&clientAddr;
//         inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
//         clientPort = ntohs(s->sin_port);
//     } else if (clientAddr.ss_family == AF_INET6) {
//         struct sockaddr_in6* s = (struct sockaddr_in6*)&clientAddr;
//         inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
//         clientPort = ntohs(s->sin6_port);
//     } else {
//         std::strcpy(ipstr, "unknown");
//     }

//     std::cout << "New client from " << ipstr << ":" << clientPort
//               << " (fd=" << clientFd << ")\n";

//     // For now, close immediately (later you keep it and add to poll/epoll)
//     close(clientFd);
// } //

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
		listenFd= socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listenFd == -1)
			continue;
		int	yes = 1;
		setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		if (bind(listenFd, p->ai_addr, p->ai_addrlen) == 0){
			if (listen(listenFd, 128) == 0){
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

	m_listenFd = createListeningSocket(m_cfg.getPort());
	std::cout << "Listening...\n";

	// while(true){
	// 	 acceptAndPrintClient(m_listenFd);
    // 		usleep(10000);
	// }
}


Server::Server(const Config &cfg) : m_cfg(cfg) {}

Server::~Server(){}