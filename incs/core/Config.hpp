#pragma once
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

class Config
{
private:
	int m_port;
	std::string m_passwd;
	Config(int port, const std::string& password);

public:
	int getPort() const;
	const std::string& getPassword() const;
	static Config checkArgs(int argc, char** argv);
};
