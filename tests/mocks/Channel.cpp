#include "Channel.hpp"
#include "Client.hpp"
#include <iostream>

ChannelMock::ChannelMock(const std::string& name)
	: m_name(name), m_topic(""), m_key(""), m_members(), m_operators(), m_invited(),
	  m_inviteOnly(false), m_topicRestricted(true), m_userLimit(-1)
{
	std::cout << "[MOCK] ChannelMock created: " << name << std::endl;
}

ChannelMock::~ChannelMock()
{
	std::cout << "[MOCK] ChannelMock destroyed: " << m_name << std::endl;
}

const std::string& ChannelMock::getName() const
{
	return m_name;
}
const std::string& ChannelMock::getTopic() const
{
	return m_topic;
}
const std::string& ChannelMock::getKey() const
{
	return m_key;
}
size_t ChannelMock::getMemberCount() const
{
	return m_members.size();
}
bool ChannelMock::isInviteOnly() const
{
	return m_inviteOnly;
}
bool ChannelMock::isTopicRestricted() const
{
	return m_topicRestricted;
}
int ChannelMock::getUserLimit() const
{
	return m_userLimit;
}

void ChannelMock::setTopic(const std::string& topic)
{
	m_topic = topic;
	std::cout << "[MOCK] Topic set for " << m_name << ": " << topic << std::endl;
}

void ChannelMock::setKey(const std::string& key)
{
	m_key = key;
	std::cout << "[MOCK] Key set for " << m_name << std::endl;
}

void ChannelMock::setInviteOnly(bool inviteOnly)
{
	m_inviteOnly = inviteOnly;
	std::cout << "[MOCK] Invite-only " << (inviteOnly ? "enabled" : "disabled") << " for " << m_name
			  << std::endl;
}

void ChannelMock::setTopicRestricted(bool restricted)
{
	m_topicRestricted = restricted;
	std::cout << "[MOCK] Topic restriction " << (restricted ? "enabled" : "disabled") << " for "
			  << m_name << std::endl;
}

void ChannelMock::setUserLimit(int limit)
{
	m_userLimit = limit;
	std::cout << "[MOCK] User limit set to " << limit << " for " << m_name << std::endl;
}

void ChannelMock::addInvite(IClient* client)
{
	m_invited.insert(client);
	std::cout << "[MOCK] " << m_name << ": Added invite" << std::endl;
}

bool ChannelMock::isInvited(IClient* client) const
{
	return m_invited.find(client) != m_invited.end();
}

bool ChannelMock::addMember(IClient* client, const std::string& key)
{
	if (!client)
		return false;
	if (m_members.find(client) != m_members.end())
		return false;
	if (m_inviteOnly && !isInvited(client))
	{
		std::cout << "[MOCK] " << m_name << ": Join denied - invite only" << std::endl;
		return false;
	}
	if (m_userLimit > 0 && static_cast< int >(m_members.size()) >= m_userLimit)
	{
		std::cout << "[MOCK] " << m_name << ": Join denied - channel full" << std::endl;
		return false;
	}
	if (!m_key.empty() && key != m_key)
	{
		std::cout << "[MOCK] " << m_name << ": Join denied - bad key" << std::endl;
		return false;
	}
	m_members.insert(client);
	m_invited.erase(client);
	if (m_members.size() == 1)
	{
		m_operators.insert(client);
		std::cout << "[MOCK] " << m_name << ": Added first member as operator" << std::endl;
	}
	else
		std::cout << "[MOCK] " << m_name << ": Added member" << std::endl;
	return true;
}

void ChannelMock::removeMember(IClient* client)
{
	m_members.erase(client);
	m_operators.erase(client);
	std::cout << "[MOCK] " << m_name << ": Removed member" << std::endl;
}

bool ChannelMock::hasMember(IClient* client) const
{
	return m_members.find(client) != m_members.end();
}

bool ChannelMock::isOperator(IClient* client) const
{
	return m_operators.find(client) != m_operators.end();
}

void ChannelMock::addOperator(IClient* client)
{
	if (hasMember(client))
	{
		m_operators.insert(client);
		std::cout << "[MOCK] " << m_name << ": Added operator" << std::endl;
	}
}

void ChannelMock::removeOperator(IClient* client)
{
	if (isOperator(client))
		m_operators.erase(client);
}

std::string ChannelMock::getMemberList() const
{
	std::string list;
	for (std::set< IClient* >::const_iterator it = m_members.begin(); it != m_members.end(); ++it)
	{
		if (it != m_members.begin())
			list += " ";
		if (isOperator(*it))
			list += "@";
		list += (*it)->getNickname();
	}
	return list;
}

void ChannelMock::broadcast(const std::string& message, IClient* exclude)
{
	std::cout << "[MOCK] " << m_name << ": Broadcasting message";
	if (exclude)
		std::cout << " (excluding one client)";
	std::cout << std::endl;
	for (std::set< IClient* >::iterator it = m_members.begin(); it != m_members.end(); ++it)
	{
		if (*it != exclude)
			(*it)->getBuffer().appendWrite(message);
	}
}

bool ChannelMock::isEmpty() const
{
	return m_members.empty();
}

IClient* ChannelMock::getMemberByNickname(const std::string& nickname)
{
	std::set< IClient* >::iterator it;

	for (it = m_members.begin(); it != m_members.end(); ++it)
	{
		IClient* member = *it;
		if (member->getNickname() == nickname)
			return (member);
	}
	return NULL;
}
