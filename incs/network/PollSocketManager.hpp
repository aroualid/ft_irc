#pragma once

#include <cstring>
#include <map>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

#include "ISocketManager.hpp"

class PollSocketManager : public ISocketManager
{
private:
	int m_epollFd;
	std::vector< epoll_event > m_events;

	PollSocketManager(const PollSocketManager&);			// copy constructor
	PollSocketManager& operator=(const PollSocketManager&); // assignement operator

public:
	PollSocketManager();
	~PollSocketManager();

	void addSocket(int fd, int events);
	void removeSocket(int fd);
	void modifySocket(int fd, int events);

	int wait(int timeout_ms);
	const std::vector< epoll_event >& getEvents() const;
};
