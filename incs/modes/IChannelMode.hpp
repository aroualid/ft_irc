#pragma once
#include "../ft_irc.hpp"

class Channel;
class Client;
class IChannelMode {
public:
    virtual ~IChannelMode() {}

    virtual bool apply(Channel* channel, bool set,
                      const std::string& param, Client* setter) = 0;
    virtual char getModeChar() const = 0;
    virtual bool requiresParam(bool set) const = 0;
    virtual bool validateParam(const std::string& param) const = 0;
    virtual std::string getState(Channel* channel) const = 0;
};
