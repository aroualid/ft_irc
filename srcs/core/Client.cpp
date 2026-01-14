#include "Client.hpp"


int	Client::getFd() const
{
	return (_fd);
}

const std::string	&Client::getNickname() const
{
	return (_nickname);
}

bool Client::isAuthenticated()const
{
	return(_passwordProvided);
}

// bool Client::isRegistered() const
// {
// 	return (_state == REGISTERED);
// }


void	Client::setNickname(const std::string &nick)
{
	_nickname = nick;
}

void	Client::setPasswordProvided(bool provided)
{
	_passwordProvided = provided;
}

// void	Client::updateRegistrationState()
// {
// 	if (!_nickname.empty() && !_username.empty())
// 		_state = REGISTERED;
// }

void	Client::joinChannel(const std::string &channel)
{
	_channels.insert(channel);
}

void	Client::leaveChannel(const std::string &channel)
{
	_channels.erase(channel);
}

bool	Client::isInChannel(const std::string &channel) const
{
	return (_channels.find(channel) != _channels.end());
}

MessageBuffer	&Client::getBuffer()
{
	return _buffer;
}


std::string Client::getPrefix() const
{
	std::string nick = _nickname.empty() ? "*" : _nickname;
	std::string user = _username.empty() ? "*" : _username;
	std::string host = _hostname.empty() ? "*" : _hostname;

	return nick + "!" + user + "@" + host;
}

Client::Client(int fd, const std::string &hostname)
	:_fd(fd),
	_nickname(""),
	_username(""),
	_realname(""),
	_hostname(hostname),
	//_state(DISCONNECTED),
	_passwordProvided(false),
	_buffer(),
	_channels()
{

}

Client::~Client() {}