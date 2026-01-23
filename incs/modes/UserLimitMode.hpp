#pragma once

#include "IChannelMode.hpp"
#include "core/IChannel.hpp"
#include "core/IClient.hpp"

class UserLimitMode : public IChannelMode {
public:
    bool apply(IChannel* channel, bool set, const std::string& param, IClient* setter);
    char getModeChar() const { return 'l'; }
    bool requiresParamToSet() const { return true; }
    bool requiresParamToUnset() const { return false; }
	bool validateParam(const std::string& param) const;
    bool isActive(IChannel* channel) const { return channel->getUserLimit() > 0; }
    std::string getParam(IChannel* channel) const;

};
