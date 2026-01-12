#pragma once
#include "Config.hpp"
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

class   Server
{
    private:
        const Config  m_cfg;
    
    public:    
        explicit    Server(const Config &cfg);
        void        run();
};