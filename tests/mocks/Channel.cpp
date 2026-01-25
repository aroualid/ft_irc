#include "Channel.hpp"
#include "Client.hpp"
#include "MockOutput.hpp"

ChannelMock::ChannelMock(const std::string& name)
	: m_name(name), m_topic(""), m_key(""), m_members(), m_operators(), m_invited(),
	  m_inviteOnly(false), m_topicRestricted(true), m_userLimit(-1)
{
	MOCK_LOG("ChannelMock created: " << name);
}

ChannelMock::~ChannelMock()
{
	MOCK_LOG("ChannelMock destroyed: " << m_name);
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
	MOCK_LOG("Topic set for " << m_name << ": " << topic);
}

void ChannelMock::setKey(const std::string& key)
{
	m_key = key;
	MOCK_LOG("Key set for " << m_name);
}

void ChannelMock::setInviteOnly(bool inviteOnly)
{
	m_inviteOnly = inviteOnly;
	MOCK_LOG("Invite-only " << (inviteOnly ? "enabled" : "disabled") << " for " << m_name);
}

void ChannelMock::setTopicRestricted(bool restricted)
{
	m_topicRestricted = restricted;
	MOCK_LOG("Topic restriction " << (restricted ? "enabled" : "disabled") << " for " << m_name);
}

void ChannelMock::setUserLimit(int limit)
{
	m_userLimit = limit;
	MOCK_LOG("User limit set to " << limit << " for " << m_name);
}

void ChannelMock::addInvite(IClient* client)
{
	m_invited.insert(client);
	MOCK_LOG(m_name << ": Added invite");
}

void ChannelMock::removeInvite(IClient* client)
{
	if (isInvited(client))
		m_invited.erase(client);
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
		MOCK_LOG(m_name << ": Join denied - invite only");
		return false;
	}
	if (m_userLimit > 0 && static_cast< int >(m_members.size()) >= m_userLimit)
	{
		MOCK_LOG(m_name << ": Join denied - channel full");
		return false;
	}
	if (!m_key.empty() && key != m_key)
	{
		MOCK_LOG(m_name << ": Join denied - bad key");
		return false;
	}
	m_members.insert(client);
	m_invited.erase(client);
	if (m_members.size() == 1)
	{
		m_operators.insert(client);
		MOCK_LOG(m_name << ": Added first member as operator");
	}
	else
		MOCK_LOG(m_name << ": Added member");
	return true;
}

void ChannelMock::removeMember(IClient* client)
{
	m_members.erase(client);
	m_operators.erase(client);
	MOCK_LOG(m_name << ": Removed member");
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
		MOCK_LOG(m_name << ": Added operator");
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
	MOCK_LOG(m_name << ": Broadcasting message" << (exclude ? " (excluding one client)" : ""));
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

bool ChannelMock::applyMode(char mode, bool set, const std::string& param, IClient* setter)
{
	(void)param;
	(void)setter;
	MOCK_LOG(m_name << ": Applying mode " << (set ? "+" : "-") << mode);
	return true;
}

std::string ChannelMock::getModeString() const
{
	std::string modes = "+";
	if (m_inviteOnly)
		modes += "i";
	if (m_topicRestricted)
		modes += "t";
	if (!m_key.empty())
		modes += "k";
	if (m_userLimit > 0)
		modes += "l";
	return modes;
}
