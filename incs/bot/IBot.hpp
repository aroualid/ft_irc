#pragma once

#include <string>

class IClient;
class IChannel;

class IBot
{
public:
	virtual ~IBot()
	{
	}

	virtual void onPrivateMessage(IClient* sender, const std::string& message) = 0;
	virtual void onChannelMessage(IClient* sender, IChannel* channel,
								  const std::string& message) = 0;
	virtual IClient* getClient() = 0;
};
