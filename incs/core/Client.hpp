#pragma once
#include "../ft_irc.hpp"

class Channel;
class ClientState;
class MessageBuffer;

class Client {
private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _hostname;

    ClientState _state;
    bool _passwordProvided;

    MessageBuffer _buffer;
    std::set<std::string> _channels;

public:
    Client(int fd, const std::string& hostname);
    ~Client();

    // Getters
    int getFd() const;
    const std::string& getNickname() const;
    bool isAuthenticated() const;
    bool isRegistered() const;

    // State management
    void setNickname(const std::string& nick);
    void setPasswordProvided(bool provided);
    void updateRegistrationState();

    // Channel membership
    void joinChannel(const std::string& channel);
    void leaveChannel(const std::string& channel);
    bool isInChannel(const std::string& channel) const;

    // Buffer access
    MessageBuffer& getBuffer();
    std::string getPrefix() const;  // "nick!user@host"
};
