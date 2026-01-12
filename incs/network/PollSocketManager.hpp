#pragma once

#include <vector>
#include <map>

class PollSocketManager : public ISocketManager {
private:
    std::vector<pollfd> _pollfds;
    std::map<int, size_t> _fdToIndex;

public:
    void addSocket(int fd, int events);
    void removeSocket(int fd);
    void modifySocket(int fd, int events);
    std::vector<SocketEvent> wait(int timeout_ms = -1);
};