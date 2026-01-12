
#include <vector>

class ISocketManager {
public:
    virtual ~ISocketManager() {}
    virtual void addSocket(int fd, int events) = 0;
    virtual void removeSocket(int fd) = 0;
    virtual void modifySocket(int fd, int events) = 0;
    virtual std::vector<SocketEvent> wait(int timeout_ms = -1) = 0;
};
