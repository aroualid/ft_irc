#pragma once
#include "core/IChannel.hpp"
#include "modes/IChannelMode.hpp"

class Channel : public IChannel
{
private:
	std::string _name;
	std::string _topic;
	std::string _key;

	std::set< IClient* > _members;
	std::set< IClient* > _operators;
	std::set< IClient* > _invited;

	bool _inviteOnly;
	bool _topicRestricted;
	int _userLimit;

	std::map< char, IChannelMode* > _modes;

public:
	Channel(const std::string& name);
	~Channel();

	// Member management
	bool addMember(IClient* client, const std::string& key = "");
	void removeMember(IClient* client);
	bool hasMember(IClient* client) const;

	void addInvite(IClient* client);
	bool isInvited(IClient* client) const;
	// Operator management
	void addOperator(IClient* client);
	bool isOperator(IClient* client) const;
	void removeOperator(IClient* client);
	// Mode application
	bool applyMode(char mode, bool set, const std::string& param, IClient* setter);
	std::string getModeString() const;

	// Broadcasting
	void broadcast(const std::string& message, IClient* exclude = NULL);
	std::string getMemberList() const;
	bool isEmpty() const;

	void setTopic(const std::string& topic);
	void setKey(const std::string& key);
	void setInviteOnly(bool inviteOnly);
	void setTopicRestricted(bool restricted);
	void setUserLimit(int limit);

	const std::string& getName() const;
	const std::string& getTopic() const;
	const std::string& getKey() const;
	size_t getMemberCount() const;
	bool isInviteOnly() const;
	bool isTopicRestricted() const;
	int getUserLimit() const;
	IClient* getMemberByNickname(const std::string& nick);
};
