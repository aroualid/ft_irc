#include "PollSocketManager.hpp"
#include <cerrno>

const std::vector< epoll_event >& PollSocketManager::getEvents() const
{
	return (m_events);
}

int PollSocketManager::wait(int timeout_ms)
{
	int n = epoll_wait(m_epollFd, &m_events[0], (int)m_events.size(), timeout_ms);

	if (n == -1 && errno != EINTR)
		throw std::runtime_error("epoll_wait failed\n");

	if (n == (int)m_events.size())
		m_events.resize(m_events.size() * 2);

	return (n);
}

void PollSocketManager::modifySocket(int fd, int events)
{
	epoll_event evt;

	std::memset(&evt, 0, sizeof(evt));

	evt.events = events;
	evt.data.fd = fd;

	if (epoll_ctl(m_epollFd, EPOLL_CTL_MOD, fd, &evt) == -1)
		throw std::runtime_error("Modification of socket failed\n");
}

void PollSocketManager::removeSocket(int fd)
{
	if (epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, 0) == -1)
		throw std::runtime_error("Deletion of socket failed\n");
}

void PollSocketManager::addSocket(int fd, int events)
{
	epoll_event evt;

	std::memset(&evt, 0, sizeof(evt));

	evt.events = events;
	evt.data.fd = fd;

	if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &evt) == -1)
		throw std::runtime_error("Creation of socket failed\n");
}

PollSocketManager::PollSocketManager() : m_epollFd(-1), m_events(64) // 64  =
{
	m_epollFd = epoll_create(64); // creation of the epoll instance.
	if (m_epollFd == -1)
		throw std::runtime_error("Instance of epoll failed to create");
}

PollSocketManager::~PollSocketManager()
{
	if (m_epollFd != -1)
		close(m_epollFd);
}