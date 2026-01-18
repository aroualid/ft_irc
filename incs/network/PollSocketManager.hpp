#pragma once

#include <vector>
#include <map>
#include <sys/epoll.h>
#include <stdexcept>
#include <unistd.h>
#include <cstring>

#include "ISocketManager.hpp"

class EpollSocketManager : public ISocketManager {
private:
	int m_epollFd;
	std::vector<epoll_event> m_events;

	EpollSocketManager(const EpollSocketManager&); //copy constructor
	EpollSocketManager &operator=(const EpollSocketManager&); //assignement operator

public:
	EpollSocketManager();
	~EpollSocketManager();

	void addSocket(int fd, int events);
	void removeSocket(int fd);
	void modifySocket(int fd, int events);
	
	int	wait(int timeout_ms);
	const std::vector<epoll_event> &getEvents() const;
};

