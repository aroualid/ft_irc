#pragma once

#include "core/IChannel.hpp"
#include <set>
#include <string>

class ChannelMock : public IChannel
{
private:
	std::string m_name;
	std::string m_topic;
	std::string m_key;
	std::set< IClient* > m_members;
	std::set< IClient* > m_operators;
	std::set< IClient* > m_invited;
	bool m_inviteOnly;
	bool m_topicRestricted;
	int m_userLimit;

public:
	ChannelMock(const std::string& name);
	virtual ~ChannelMock();

	const std::string& getName() const;
	const std::string& getTopic() const;
	const std::string& getKey() const;
	size_t getMemberCount() const;
	bool isInviteOnly() const;
	bool isTopicRestricted() const;
	int getUserLimit() const;

	void setTopic(const std::string& topic);
	void setKey(const std::string& key);
	void setInviteOnly(bool inviteOnly);
	void setTopicRestricted(bool restricted);
	void setUserLimit(int limit);

	void addInvite(IClient* client);
	bool isInvited(IClient* client) const;
	void removeInvite(IClient* client);

	bool addMember(IClient* client, const std::string& key = "");
	void removeMember(IClient* client);
	bool hasMember(IClient* client) const;

	bool isOperator(IClient* client) const;
	void addOperator(IClient* client);
	void removeOperator(IClient* client);

	std::string getMemberList() const;
	void broadcast(const std::string& message, IClient* exclude = NULL);
	bool isEmpty() const;
};
