#pragma once
#include "../ft_irc.hpp"


class Channel {
private:
    std::string _name;
    std::string _topic;
    std::string _key;

    std::set<Client*> _members;
    std::set<Client*> _operators;
    std::set<Client*> _invited;

    bool _inviteOnly;
    bool _topicRestricted;
    int _userLimit;

    std::map<char, IChannelMode*> _modes;

public:
    Channel(const std::string& name);
    ~Channel();

    // Member management
    bool addMember(Client* client, const std::string& key = "");
    void removeMember(Client* client);
    bool hasMember(Client* client) const;

    // Operator management
    void addOperator(Client* client);
    bool isOperator(Client* client) const;

    // Mode application
    bool applyMode(char mode, bool set, const std::string& param, Client* setter);
    std::string getModeString() const;

    // Broadcasting
    void broadcast(const std::string& message, Client* exclude = NULL);
    std::string getMemberList() const;
};
