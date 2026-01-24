#pragma once

#include "IChannel.hpp"
#include <set>
#include <string>

class IMessageBuffer;
// class MessageBuffer;

class IClient
{
public:
	virtual ~IClient()
	{
	}

	virtual int getFd() const = 0;
	virtual const std::string& getNickname() const = 0;
	virtual const std::string& getUsername() const = 0;
	virtual const std::string& getRealname() const = 0;
	virtual const std::string& getHostname() const = 0;

	virtual bool isPasswordProvided() const = 0;
	virtual bool isRegistered() const = 0;

	virtual void setNickname(const std::string& nick) = 0;
	virtual void setUsername(const std::string& user) = 0;
	virtual void setRealname(const std::string& real) = 0;
	virtual void setPasswordProvided(bool provided) = 0;
	virtual void attemptRegistration() = 0;

	virtual void joinChannel(IChannel* channel) = 0;
	virtual void leaveChannel(IChannel* channel) = 0;
	virtual bool isInChannel(const std::string& channel) const = 0;
	virtual const std::set< IChannel* >& getChannels() const = 0;

	virtual IMessageBuffer& getBuffer() = 0;
	virtual const IMessageBuffer& getBuffer() const = 0;

	// MessageBuffer& getBuffer();

	virtual std::string getPrefix() const = 0;
};
