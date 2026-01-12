#pragma once
#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <stdexcept>
#include <cstring>

class Config
{
    private:
        int         m_port;
        std::string m_passwd;
        Config(int port, std::string password);

    public:
        int                 getPort() const;
        const std::string	&getPasswd() const;
        static Config       checkArgs(int argc, char **argv);
};