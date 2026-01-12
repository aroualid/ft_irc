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
#include "ISocketManager.hpp"
#include <netdb.h> 
#include <sstream>

class   Server
{
	private:
        const Config    m_cfg;
        ISocketManager  *m_sm;
        int             m_listenFd;

		void			setupListeningsocket();
		void			handleEvent(const SocketEvent &evt);
        
        
        public:
        Server(const Config &cfg);
        ~Server();
        void        run();
        //void        resolveAddresses();
};