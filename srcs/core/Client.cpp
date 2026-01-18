#include "Client.hpp"

int	Client::getFd() const
{
	return (_fd);
}

const std::string& Client::getNickname() const
{
	return (_nickname);
}

Client::Client(int fd, const std::string &hostname)
	:_fd(fd),
	_nickname(""),
	_username(""),
	_realname(""),
	_hostname(hostname),
	_state(DISCONNECTED),
	_passwordProvided(false),
	_buffer(),
	_channels()
{}

Client::~Client() {}
const std::string& Client::getUsername() const
{
	return (_username);
}

const std::string& Client::getRealname() const
{
	return (_realname);
}

bool Client::isAuthenticated() const
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
	updateRegistrationState();
}

void Client::setUsername(const std::string& user)
{
	_username = user;
	updateRegistrationState();
}

void Client::setRealname(const std::string& real)
{
	_realname = real;
	updateRegistrationState();
}

void Client::setPasswordProvided(bool provided)
{
	_passwordProvided = provided;
	updateRegistrationState();
}

void Client::joinChannel(const std::string& channel) {
    _channels.insert(channel);
}

void Client::leaveChannel(const std::string& channel) {
    _channels.erase(channel);
}

bool Client::isInChannel(const std::string& channel) const {
    return _channels.find(channel) != _channels.end();
}

void Client::updateRegistrationState()
{
	if (_state == REGISTERED)
		return;
	if (_passwordProvided && !_nickname.empty() && !_username.empty()) {
        _state = REGISTERED;
	}
}

MessageBuffer& Client::getBuffer()
{
	return (_buffer);
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
