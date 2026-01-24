#pragma once

#include "IChannelMode.hpp"
#include "core/IChannel.hpp"
#include "core/IClient.hpp"

class InviteOnlyMode : public IChannelMode
{
public:
	bool apply(IChannel* channel, bool set, const std::string& param, IClient* setter);
	char getModeChar() const
	{
		return 'i';
	}
	bool requiresParamToSet() const
	{
		return false;
	}
	bool requiresParamToUnset() const
	{
		return false;
	}
	bool validateParam(const std::string& param) const;
	bool isActive(IChannel* channel) const
	{
		return channel->isInviteOnly();
	}
	std::string getParam(IChannel* channel) const
	{
		(void)channel;
		return "";
	}
};
