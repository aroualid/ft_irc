#pragma once

#include <string>

class IClient;
class IChannel;

class IServer
{
public:
	virtual ~IServer()
	{
	}

	virtual int getPort() const = 0;
	virtual const std::string& getPassword() const = 0;
	virtual bool requiresPassword() const = 0;

	virtual IClient* getClientByNickname(const std::string& nick) = 0;
	virtual void registerClient(const std::string& nick, IClient* client) = 0;
	virtual void unregisterClient(const std::string& nick) = 0;

	virtual IChannel* getChannel(const std::string& name) = 0;
	virtual IChannel* createChannel(const std::string& name, IClient* creator) = 0;
	virtual void deleteChannelIfEmpty(IChannel* channel) = 0;
	virtual size_t getChannelCount() const = 0;
	virtual std::string getServerName() const = 0;
};
