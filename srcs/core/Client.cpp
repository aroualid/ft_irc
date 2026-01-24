#include "core/Client.hpp"
#include "IChannel.hpp"
#include "core/IServer.hpp"

Client::Client(int fd, const std::string& hostname, IServer& server)
	: _fd(fd), _nickname(""), _username(""), _realname(""), _hostname(hostname), _state(HANDSHAKE),
	  _passwordProvided(false), _buffer(), _server(&server), _channels()
{
}

Client::~Client()
{
}

int Client::getFd() const
{
	return _fd;
}

const std::string& Client::getNickname() const
{
	return (_nickname);
}

const std::string& Client::getUsername() const
{
	return (_username);
}

const std::string& Client::getRealname() const
{
	return (_realname);
}

const std::string& Client::getHostname() const
{
	return (_hostname);
}

bool Client::isPasswordProvided() const
{
	return (_passwordProvided);
}

bool Client::isRegistered() const
{
	return (_state == REGISTERED);
}

void Client::setNickname(const std::string& nick)
{
	_nickname = nick;
	attemptRegistration();
}

void Client::setUsername(const std::string& user)
{
	_username = user;
	attemptRegistration();
}

void Client::setRealname(const std::string& real)
{
	_realname = real;
}

void Client::setPasswordProvided(bool provided)
{
	_passwordProvided = provided;
	attemptRegistration();
}

void Client::joinChannel(IChannel* channel)
{
	_channels.insert(channel);
}

void Client::leaveChannel(IChannel* channel)
{
	_channels.erase(channel);
}

bool Client::isInChannel(const std::string& channelName) const
{
	for (std::set< IChannel* >::const_iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if ((*it)->getName() == channelName)
			return true;
	}
	return false;
}

const std::set< IChannel* >& Client::getChannels() const
{
	return _channels;
}

void Client::attemptRegistration()
{
	if (_state == REGISTERED)
		return;
	if (_nickname.empty())
		return;
	if (_username.empty())
		return;
	if (_server->requiresPassword() && !_passwordProvided)
		return;
	_state = REGISTERED;
}

IMessageBuffer& Client::getBuffer()
{
	return _buffer;
}

const IMessageBuffer& Client::getBuffer() const
{
	return _buffer;
}

std::string Client::getPrefix() const
{
	if (_nickname.empty())
		return _hostname;

	std::string prefix = _nickname;
	if (!_username.empty())
		prefix += "!" + _username + "@" + _hostname;
	return prefix;
}
