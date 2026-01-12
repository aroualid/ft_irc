#include "Config.hpp"

std::string parsePasswd(char *argv)
{
    if (!argv)
        throw(std::runtime_error("Password must not be null\n."));
    std::string passwd = argv;
    if (passwd.empty() || (*argv == ' ' && strlen(argv) == 1))
        throw(std::runtime_error("Password must not be empty\n."));
    return (passwd);
}

int     parsePort(char *argv)
{
    if (!argv || *argv == '\0')
        throw(std::runtime_error("Port must not be empty\n."));

    errno = 0;
    char    *end = 0;
    long    port = std::strtol(argv, &end, 10);

    if (errno != 0 || end == argv || *end != '\0')
        throw(std::runtime_error("Port must be a valid number\n."));
    if (port < 1 || port > 65535)
        throw(std::runtime_error("Port must be between 1 and 65535."));
    return (static_cast<int>(port));
}

Config  Config::checkArgs(int argc, char **argv)
{
    if (argc != 3)
        throw(std::runtime_error("Number of arguments must be equal to 3\n."));
        
    int port = parsePort(argv[1]);
    std::string passwd = parsePasswd(argv[2]);

    return(Config(port, passwd));
}

/*getters*/
int Config::getPort() const
{
    return this->m_port;
}

const std::string &Config::getPasswd() const
{
    return this->m_passwd;
}

Config::Config(int port, std::string password) : m_port(port), m_passwd(password) {}