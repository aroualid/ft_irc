#pragma once
#include "../ft_irc.hpp"
#include "../core/IClient.hpp"
#include "../core/IChannel.hpp"

class IChannelMode;
 
class Channel: public IChannel {
private:
    std::string _name;
    std::string _topic;
    std::string _key;

    std::set<IClient*> _members;
    std::set<IClient*> _operators;
    std::set<IClient*> _invited;

    bool _inviteOnly;
    bool _topicRestricted;
    int _userLimit;

    std::map<char, IChannelMode*> _modes;

public:
    Channel(const std::string& name);
    ~Channel();

    // Member management
    bool addMember(IClient* client, const std::string& key = "");
    void removeMember(IClient* client);
    bool hasMember(IClient* client) const;

    // Operator management
    void addOperator(IClient* client);
    bool isOperator(IClient* client) const;

    // Mode application
    bool applyMode(char mode, bool set, const std::string& param, IClient* setter);
    std::string getModeString() const;

    // Broadcasting
    void broadcast(const std::string& message, IClient* exclude = NULL);
    std::string getMemberList() const;
};
