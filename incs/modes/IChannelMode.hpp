#pragma once
#include "../ft_irc.hpp"
class IClient;
class IChannel;
class IChannelMode
{
public:
	virtual ~IChannelMode()
	{
	}

	virtual bool apply(IChannel* channel, bool set, const std::string& param, IClient* setter) = 0;
	virtual char getModeChar() const = 0;
	virtual bool requiresParamToSet() const = 0;   // +mode needs param?
	virtual bool requiresParamToUnset() const = 0; // -mode needs param?
	virtual bool validateParam(const std::string& param) const = 0;
	virtual bool isActive(IChannel* channel) const = 0;		   // is mode currently set?
	virtual std::string getParam(IChannel* channel) const = 0; // "" if no param
};
