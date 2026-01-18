#pragma once

#include <set>
#include <string>

class IMessageBuffer;

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
	virtual bool isAuthenticated() const = 0;

	virtual void setNickname(const std::string& nick) = 0;
	virtual void setUsername(const std::string& user) = 0;
	virtual void setRealname(const std::string& real) = 0;
	virtual void setPasswordProvided(bool provided) = 0;
	virtual void updateRegistrationState() = 0;

	virtual void joinChannel(const std::string& channel) = 0;
	virtual void leaveChannel(const std::string& channel) = 0;
	virtual bool isInChannel(const std::string& channel) const = 0;
	virtual const std::set< std::string >& getChannels() const = 0;

	virtual IMessageBuffer& getBuffer() = 0;
	virtual const IMessageBuffer& getBuffer() const = 0;

	virtual std::string getPrefix() const = 0;
};
