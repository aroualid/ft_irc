#pragma once
#include <iostream>

class	Server {
	public:
		~Server();
		void	run();
		Server *init(int port, const std::string &password);

	private:
		Server(int port, const std::string &password);
		int	m_port;
		std::string	m_password;

		int	m_listeningSocketFd;

};

class	Client {
	public:

	private:
};

class	Command {
	public:

	private:
};

Server *Server::init(int port, const std::string &password)
{
	//on fait nos checks ici
	//if c'est good
		return (new Server(port, password));
	//else si c'est de la merde
		return (NULL);
}