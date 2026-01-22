#pragma once
#include "../ft_irc.hpp"
#include "../core/IMessageBuffer.hpp"
#include "IClient.hpp"
#include "IChannel.hpp"
class ClientState;

class Client : public IClient{
private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _hostname;

	enum ClientState {
	    HANDSHAKE,
	    LOGIN,
	    REGISTERED,
	    DISCONNECTED
	};
    ClientState _state;
    bool _passwordProvided;

    //IMessageBuffer _buffer;
    std::set<std::string> _channels;


public:
    Client(int fd, const std::string& hostname);
    ~Client();

    // Getters
    int getFd() const;
    const std::string& getNickname() const;
    const std::string& getUsername() const;
    const std::string& getRealname() const;
	const std::string& getHostname() const;


	bool isPasswordProvided() const;
    bool isAuthenticated() const;
    bool isRegistered() const;

    // State management
    void setNickname(const std::string& nick);
    void setUsername(const std::string& user);
    void setRealname(const std::string& real);
    void setPasswordProvided(bool provided);
    void updateRegistrationState();

    // Channel membership
    void joinChannel(const std::string& channel);
    void leaveChannel(const std::string& channel);
    bool isInChannel(const std::string& channel) const;
	const std::set< std::string >& getChannels() const;
    // Buffer access
    IMessageBuffer& getBuffer();
    const IMessageBuffer& getBuffer() const;
    std::string getPrefix() const;  // "nick!user@host"
};
