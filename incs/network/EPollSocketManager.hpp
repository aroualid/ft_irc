#pragma once

#include <vector>
#include <map>
#include <sys/epoll.h>
#include "ISocketManager.hpp"

class EPollSocketManager : public ISocketManager {
private:
	int m_epollFd;
	std::vector<epoll_event> m_events;

public:
	EPollSocketManager();
	~EPollSocketManager();

	void addSocket(int fd, int events);
	void removeSocket(int fd) ;
	void modifySocket(int fd, int events);
	std::vector<SocketEvent> wait(int timeout_ms = -1) ;
};

