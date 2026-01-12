#pragma once

#include "IChannelMode.hpp"

class UserLimitMode : public IChannelMode {
public:
    bool apply(Channel* channel, bool set, const std::string& param, Client* setter);
    char getModeChar() const { return 'i'; }
    bool requiresParam(bool set) const { return false; }
};
