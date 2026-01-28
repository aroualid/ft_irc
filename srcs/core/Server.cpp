#include "core/Server.hpp"
#include "core/Channel.hpp"
#include "core/Client.hpp"
#include "core/IClient.hpp"
#include "core/IMessageBuffer.hpp"
#include "network/MessageBuffer.hpp"
#include "network/PollSocketManager.hpp"

#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "protocol/Message.hpp"
#include "protocol/MessageParser.hpp"
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <sstream>
#include <stdexcept>

extern volatile sig_atomic_t g_shutdown;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Server::getPort() const
{
	return m_cfg.getPort();
}

const std::string& Server::getPassword() const
{
	return m_cfg.getPassword();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// getClientByNickname
// register
// unregister

void Server::deleteChannelIfEmpty(IChannel* channel)
{
	if (!channel)
		return;
	if (!channel->isEmpty())
		return;
	m_channels.erase(channel->getName());
	delete channel;
}

void Server::checkClientTimeouts()
{
	std::time_t now = std::time(NULL);
	std::vector< int > to_disconnect;

	for (std::map< int, IClient* >::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		int client_fd = it->first;
		IClient* client = it->second;

		if (client_fd == m_listenFd)
			continue;

		std::time_t last_ping = client->getLastPingSent();
		std::time_t last_activity = client->getLastActivity();

		if (last_ping != 0 && (now - last_ping) > PONG_TIMEOUT)
		{
			std::cout << "Client #" << client_fd << " didn't respond to ping. Killing it :D\n";
			std::string error_msg = "ERROR :You didn't respond to my ping and that's mean :( "
									"Therefore you're gonna die :D\r\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), MSG_NOSIGNAL);
			to_disconnect.push_back(client_fd);
			continue;
		}
		else if (last_ping == 0 && (now - last_activity) > PING_TIMEOUT)
		{
			std::cout << "Client #" << client_fd << " has been idle for " << (now - last_activity)
					  << " seconds. Checking if it's still alive lol\n";
			std::ostringstream ping;
			ping << "PING :" << getServerName() << "\r\n";
			client->getBuffer().appendWrite(ping.str());
			client->setLastPingSent(now);
			m_sm->modifySocket(client_fd, EPOLLIN | EPOLLOUT);
		}
	}

	for (size_t i = 0; i < to_disconnect.size(); i++)
		disconnectClient(to_disconnect[i]);
}

IChannel* Server::createChannel(const std::string& name, IClient* creator)
{
	IChannel* existing = getChannel(name);
	if (existing)
		return (existing);

	Channel* channel = new Channel(name);
	m_channels[name] = channel;

	channel->addMember(creator, "");
	return (channel);
}

IClient* Server::getClientByNickname(const std::string& nick)
{
	std::map< std::string, IClient* >::iterator it = m_clientsByNick.find(nick);
	if (it != m_clientsByNick.end())
	{
		return (it->second);
	}
	return (0);
}

void Server::registerClient(const std::string& nick, IClient* client)
{
	if (!nick.empty())
	{
		m_clientsByNick[nick] = client;
	}
}

void Server::unregisterClient(const std::string& nick)
{
	m_clientsByNick.erase(nick);
}

IChannel* Server::getChannel(const std::string& name)
{
	std::map< std::string, IChannel* >::iterator it =
		m_channels.find(name); // returns name of the wanted channel
	if (it != m_channels.end())
		return (it->second);
	return (0); // nullptr
}

size_t Server::getChannelCount() const
{
	return (m_channels.size());
}

std::string Server::getServerName() const
{
	return ("ircserv");
}

IClient* Server::getClient(int fd)
{
	std::map< int, IClient* >::iterator it = m_clients.find(fd);
	if (it == m_clients.end())
		return (0);
	return (it->second);
}

void Server::onIrcLine(int fd, const std::string& line)
{
	// std::cout << "fd=" << fd << " IRC line: [" << line << "]\n";
	IClient* client = getClient(fd);
	if (!client)
		return;

	Message msg = MessageParser::parse(line);
	if (!msg.isValid())
		return;

	ACommand* cmd = CommandFactory::getInstance().createCommand(msg.m_command_type, *this);
	if (!cmd)
	{
		std::cout << "[DEBUG] Command not found: " << msg.m_command << ". Ignoring...\n";
		return;
	}
	std::cout << "[DEBUG] Running " << cmd->getName() << "\n";
	cmd->execute(client, msg);
	delete cmd;

	for (std::map< int, IClient* >::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		if (!it->second->getBuffer().getWriteBuffer().empty())
			m_sm->modifySocket(it->first, EPOLLIN | EPOLLOUT);
	}
	// cmdHandle(fd, line);
}

static void setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Function fcntl with command F_GETFL failed\n");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Function fcntl with command F_SETFL failed\n");
}

void Server::disconnectClient(int fd)
{
	try
	{
		m_sm->removeSocket(fd);
	}
	catch (...)
	{ // catch any exception "..."
	}
	close(fd);
	std::map< int, IClient* >::iterator it = m_clients.find(fd);
	if (it != m_clients.end())
	{
		IClient* client = it->second;

		unregisterClient(client->getNickname());

		const std::set< IChannel* >& channels = client->getChannels();
		std::set< IChannel* > channels_copy = channels;

		for (std::set< IChannel* >::iterator chan_it = channels_copy.begin();
			 chan_it != channels_copy.end(); ++chan_it)
		{
			(*chan_it)->removeMember(client);
		}

		delete client;
		m_clients.erase(it);
	}
	std::cout << "Disconnected : " << fd << "\n";
}

void Server::writeClientsData(int fd)
{
	IClient* client = getClient(fd);
	if (!client)
	{
		disconnectClient(fd);
		return;
	}
	const std::string& out = client->getBuffer().getWriteBuffer();
	if (out.empty())
	{
		m_sm->modifySocket(fd, EPOLLIN);
		return;
	}

	ssize_t sent = send(fd, out.data(), out.size(), MSG_NOSIGNAL);

	if (sent > 0)
	{
		client->getBuffer().consumeWriteBuffer((size_t)sent);
		if (client->getBuffer().getWriteBuffer().empty())
			m_sm->modifySocket(fd, EPOLLIN);
		else
			m_sm->modifySocket(fd, EPOLLIN | EPOLLOUT);
	}

	else if (sent == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		disconnectClient(fd);
	}
}

void Server::readClientsData(int fd)
{
	IClient* client = getClient(fd);
	if (!client)
	{
		disconnectClient(fd);
		return;
	}

	char buffer[4096];
	while (true)
	{
		ssize_t receiving = recv(fd, buffer, sizeof(buffer), 0);
		if (receiving > 0)
		{
			client->updateLastActivity();
			client->getBuffer().appendRead(std::string(buffer, receiving));
			while (client->getBuffer().hasCompleteMessage())
			{
				std::string line = client->getBuffer().getNextMessage();
				onIrcLine(fd, line);
			}
			if (client->getBuffer().getReadBufferSize() > 65536)
			{
				std::cout << "Input buffer is too big, disconnecting : " << fd << "\n";
				disconnectClient(fd);
				return;
			}
		}
		else if (receiving == 0)
		{
			disconnectClient(fd);
			return;
		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return;
			disconnectClient(fd);
			return;
		}
	}
}

void Server::acceptNewClients()
{
	while (true)
	{
		int clientFd = accept(m_listenFd, 0, 0);
		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			throw std::runtime_error("Acceptation of a New Client failed\n");
		}
		setNonBlocking(clientFd);
		m_sm->addSocket(clientFd, EPOLLIN);

		m_clients[clientFd] = new Client(clientFd, "unknown", *this);
		std::cout << "Client Accepted : " << clientFd << "\n";
	}
}

void Server::handleDisconnections(int fd, unsigned int evt)
{
	std::cout << "handleDisconenctions loop ready to call\n" << fd << evt;
	if (fd == m_listenFd)
		throw std::runtime_error("Listening Socket disconnected\n");
	disconnectClient(fd);
}

bool Server::requiresPassword() const
{
	return !m_cfg.getPassword().empty();
}

static int createListeningSocket(int port) // fd that listens to all interfaces trying to bind
{
	std::stringstream ss;
	ss << port;

	std::string portStr = ss.str();
	struct addrinfo hints;
	struct addrinfo* res = 0;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // indicates that getaddrinfo() should return socket addresses for
								 // any address family (either IPv4 or IPv6, for example)
	hints.ai_socktype = SOCK_STREAM; // tcp
	hints.ai_flags = AI_PASSIVE;	 // for all interfaces to bind to server (host == NULL)

	int err = getaddrinfo(0, portStr.c_str(), &hints, &res);
	if (err != 0)
		throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(err));

	int listenFd = -1; // listening fd not created yet
	for (struct addrinfo* p = res; p != 0; p = p->ai_next)
	{
		listenFd =
			socket(p->ai_family, p->ai_socktype, p->ai_protocol); // creating the listening socket
		if (listenFd == -1)
			continue;
		int yes = 1;
		setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)); //
		if (bind(listenFd, p->ai_addr, p->ai_addrlen) == 0)
		{
			if (listen(listenFd, 128) == 0)
			{
				freeaddrinfo(res);
				return (listenFd);
			}
		}
		close(listenFd); // useful ?
		listenFd = -1;
	}
	freeaddrinfo(res);
	throw std::runtime_error("Failed to bind/listen on port : " + portStr);
}

void Server::run()
{
	std::cout << "Ready to run !\n";

	m_listenFd = createListeningSocket(m_cfg.getPort()); // the "door" of the server irc
	std::cout << "Listening...\n";

	m_sm = new PollSocketManager(); // THE Manager of the Sockets

	setNonBlocking(m_listenFd);

	m_sm->addSocket(m_listenFd, EPOLLIN); // pending incoming connexions :

	while (!g_shutdown)
	{
		int n = m_sm->wait(EPOLL_TIMEOUT_MS); // waiting for incoming connexxions

		if (g_shutdown)
			break;

		checkClientTimeouts();
		if (n < 0)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("epoll_wait failed");
		}

		const std::vector< epoll_event >& evts = m_sm->getEvents(); // vector getting events

		for (int i = 0; i < n; i++)
		{
			int fd = evts[i].data.fd;
			unsigned int evt = evts[i].events;

			if (evt & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
			{
				handleDisconnections(fd, evt);
				continue;
			}
			if (fd == m_listenFd && (evt & EPOLLIN)) // new connexions
			{
				acceptNewClients();
			}
			else
			{
				if (evt & EPOLLIN) // clients sent data
				{
					readClientsData(fd);
				}
				if (evt & EPOLLOUT)
					writeClientsData(fd);
			}
		}
	}

	if (g_shutdown)
		std::cout << "\nServer shutting down...\n";
}

Server::Server(const Config& cfg) : m_cfg(cfg), m_listenFd(-1), m_sm(0)
{
}

Server::~Server()
{
	std::cout << "Cleaning up my stuff...\n";

	for (std::map< int, IClient* >::iterator it = m_clients.begin(); it != m_clients.end(); it++)
	{
		if (it->first != m_listenFd)
		{
			std::string goodbye =
				"ERROR :Server shutting down. Say goodbye. (sike it's too late :p)\r\n";
			send(it->first, goodbye.c_str(), goodbye.length(), MSG_NOSIGNAL);
		}
		close(it->first);
		delete (it->second);
	}
	m_clients.clear();

	for (std::map< std::string, IChannel* >::iterator it = m_channels.begin();
		 it != m_channels.end(); ++it)
		delete it->second;
	m_channels.clear();

	if (m_listenFd != -1)
		close(m_listenFd);

	delete (m_sm);

	std::cout << "All resources freed. I can die in peace! :D\n";
}
