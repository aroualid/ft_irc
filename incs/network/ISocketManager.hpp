
#include <vector>
#include "EpollSocketManager.hpp"

struct SocketEvent{
	int fd;
	int event;
};

class ISocketManager {
public:
	virtual 		~ISocketManager() {}
	virtual void 	addSocket(int fd, int events) = 0;
	virtual void 	removeSocket(int fd) = 0;
	virtual void 	modifySocket(int fd, int events) = 0;

	virtual int		wait(int timeout_ms) = 0;
	virtual const	std::vector<epoll_event> &getEvents() const = 0;
};
