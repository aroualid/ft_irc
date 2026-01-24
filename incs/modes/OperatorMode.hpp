#pragma once

#include "IChannelMode.hpp"
#include "core/IChannel.hpp"
#include "core/IClient.hpp"

class OperatorMode : public IChannelMode
{
public:
	bool apply(IChannel* channel, bool set, const std::string& param, IClient* setter);
	char getModeChar() const
	{
		return 'o';
	}
	bool requiresParamToSet() const
	{
		return true;
	}
	bool requiresParamToUnset() const
	{
		return true;
	}
	bool validateParam(const std::string& param) const;
	bool isActive(IChannel* channel) const
	{
		(void)channel;
		return false;
	}
	std::string getParam(IChannel* channel) const
	{
		(void)channel;
		return "";
	}
};
