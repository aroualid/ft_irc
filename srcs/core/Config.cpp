#include "Config.hpp"
#include <sstream>

#define PORT_MIN 1024
#define PORT_MAX 65535

std::string parsePasswd(char* argv)
{
	if (!argv)
		throw(std::runtime_error("Password must not be null"));
	std::string passwd = argv;
	if (passwd.empty() || (*argv == ' ' && strlen(argv) == 1))
		throw(std::runtime_error("Password must not be empty"));
	return (passwd);
}

int parsePort(char* argv)
{
	if (!argv || *argv == '\0')
		throw(std::runtime_error("Port must not be empty"));

	errno = 0;
	char* end = 0;
	long port = std::strtol(argv, &end, 10);

	if (errno != 0 || end == argv || *end != '\0')
	{
		std::ostringstream oss;
		oss << "Port must be a number.\nI know it's crazy, but there's nothing to listen to on " << argv << " :o";
		throw(std::runtime_error(oss.str()));
	}
	if (port < PORT_MIN || port > PORT_MAX)
	{
		std::ostringstream oss;
		oss << "Port must be " << PORT_MIN << " and " << PORT_MAX;
		throw(std::runtime_error(oss.str()));
	}
	return (static_cast< int >(port));
}

Config Config::checkArgs(int argc, char** argv)
{
	if (argc != 3)
	{
		std::ostringstream oss;
		oss << "Usage: " << argv[0] << " <port> <password>";
		throw (std::runtime_error(oss.str()));
	}

	int port = parsePort(argv[1]);
	std::string passwd = parsePasswd(argv[2]);

	return (Config(port, passwd));
}

/*getters*/
int Config::getPort() const
{
	return this->m_port;
}

const std::string& Config::getPassword() const
{
	return this->m_passwd;
}

Config::Config(int port, const std::string &password) : m_port(port), m_passwd(password)
{
}
