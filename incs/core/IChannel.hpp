#pragma once

#include <string>
#include <vector>

class IClient;

class IChannel
{
public:
	virtual ~IChannel()
	{
	}

	virtual const std::string& getName() const = 0;
	virtual const std::string& getTopic() const = 0;
	virtual const std::string& getKey() const = 0;
	virtual size_t getMemberCount() const = 0;
	virtual bool isInviteOnly() const = 0;
	virtual bool isTopicRestricted() const = 0;
	virtual int getUserLimit() const = 0;

	virtual void setTopic(const std::string& topic) = 0;
	virtual void setKey(const std::string& key) = 0;
	virtual void setInviteOnly(bool inviteOnly) = 0;
	virtual void setTopicRestricted(bool restricted) = 0;
	virtual void setUserLimit(int limit) = 0;

	virtual void addInvite(IClient* client) = 0;
	virtual bool isInvited(IClient* client) const = 0;
	virtual void removeInvite(IClient* client) = 0;

	virtual bool addMember(IClient* client, const std::string& key = "") = 0;
	virtual void removeMember(IClient* client) = 0;
	virtual bool hasMember(IClient* client) const = 0;

	virtual bool isOperator(IClient* client) const = 0;
	virtual void addOperator(IClient* client) = 0;
	virtual void removeOperator(IClient* client) = 0;

	virtual std::string getMemberList() const = 0;
	virtual std::vector< IClient* > getMembers() const = 0;
	virtual void broadcast(const std::string& message, IClient* exclude = NULL) = 0;
	virtual bool isEmpty() const = 0;

	virtual bool applyMode(char mode, bool set, const std::string& param, IClient* setter) = 0;
	virtual std::string getModeString() const = 0;
};
