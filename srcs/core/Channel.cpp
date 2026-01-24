#include "core/Channel.hpp"
#include "core/Client.hpp"
#include "core/IClient.hpp"
#include "core/IMessageBuffer.hpp"
#include "modes/IChannelMode.hpp"
#include "modes/InviteOnlyMode.hpp"
#include "modes/KeyMode.hpp"
#include "modes/OperatorMode.hpp"
#include "modes/TopicRestrictedMode.hpp"
#include "modes/UserLimitMode.hpp"
#include "network/MessageBuffer.hpp"
#include <cstdlib>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <vector>

Channel::Channel(const std::string& name)
	: _name(name), _topic(""), _key(""), _inviteOnly(false), _topicRestricted(false), _userLimit(-1)
{
	_modes['i'] = new InviteOnlyMode();
	_modes['k'] = new KeyMode();
	_modes['o'] = new OperatorMode();
	_modes['t'] = new TopicRestrictedMode();
	_modes['l'] = new UserLimitMode();
}

Channel::~Channel()
{
	_members.clear();
	_operators.clear();
	_invited.clear();
	std::map< char, IChannelMode* >::iterator it = _modes.begin();
	while (it != _modes.end())
	{
		delete it->second;
		++it;
	}
	_modes.clear();
}

bool Channel::addMember(IClient* client, const std::string& key)
{
	if (hasMember(client) == true)
		return false;
	if (!_key.empty() && _key != key)
		return false;
	if (_userLimit != -1 && (int)_members.size() == _userLimit)
		return false;
	if (_inviteOnly == true)
	{
		if (_invited.find(client) == _invited.end())
			return false;
	}
	_members.insert(client);
	_invited.erase(client);
	if (_members.size() == 1)
	{
		_operators.insert(client);
	}
	return true;
}

void Channel::removeMember(IClient* client)
{
	_members.erase(client);
}

void Channel::addInvite(IClient* client)
{
	_invited.insert(client);
}

void Channel::removeInvite(IClient* client)
{
	if (isInvited(client))
		_invited.erase(client);
}

bool Channel::isInvited(IClient* client) const
{
	return (_invited.find(client) != _invited.end());
}

bool Channel::isOperator(IClient* client) const
{
	return (_operators.find(client) != _operators.end());
}

bool Channel::hasMember(IClient* client) const
{
	return (_members.find(client) != _members.end());
}

void Channel::addOperator(IClient* client)
{
	if (hasMember(client))
		_operators.insert(client);
}

void Channel::removeOperator(IClient* client)
{
	if (isOperator(client))
		_operators.erase(client);
}

bool Channel::applyMode(char mode, bool set, const std::string& param, IClient* setter)
{
	if (!isOperator(setter))
		return false;

	std::map< char, IChannelMode* >::iterator it = _modes.find(mode);

	if (it == _modes.end())
	{
		return false;
	}

	IChannelMode* modeHandler = it->second;

	if (set && modeHandler->requiresParamToSet() && param.empty())
	{
		return false;
	}

	if (!set && modeHandler->requiresParamToUnset() && param.empty())
	{
		return false;
	}

	return modeHandler->apply(this, set, param, setter);
}
/*	if (!isOperator(setter))
		return false;
	if (set == true)
	{
		switch (mode)
		{
			case 'i':
				_inviteOnly = true;
				return true;
			case 't':
				_topicRestricted = true;
				return true;
			case 'k':
				if (param.empty())
					return false;
				_key = param;
				return true;
			case 'o':
				if (hasMember(setter) == false)
					return false;
				addOperator(setter);
				return true;
			case 'l':
				if (param.empty())
					return false;
				_userLimit = std::atoi(param.c_str());
				return true;
			default : return false;
		}
	}
	if (set == false)
	{
		switch(mode)
		{
			case 'i':
				_inviteOnly = false;
				return true;
			case 't':
				_topicRestricted = false;
				return true;
			case 'k':
				_key = "";
				return true;
			case 'o':
				if (hasMember(setter) == false)
					return false;
				removeOperator(setter);
				return true;
			case 'l':
				_userLimit = -1;
				return true;
			default : return false;
		}
	}
	return false;
}*/

std::string Channel::getModeString() const
{
	std::string modes = "";
	if (!_inviteOnly && !_topicRestricted && _key.empty() && _userLimit == -1)
		return (modes);
	modes = "+";
	if (_inviteOnly)
		modes += "i";
	if (_topicRestricted)
		modes += "t";
	if (!_key.empty())
		modes += "k";
	if (_userLimit != -1)
		modes += "l";
	if (!_key.empty())
		modes += " " + _key;
	if (_userLimit != -1)
	{
		std::ostringstream oss;
		oss << _userLimit;
		std::string str = oss.str();
		modes += " " + str;
	}

	return modes;
}

void Channel::broadcast(const std::string& message, IClient* exclude)
{
	std::set< IClient* >::iterator it;

	for (it = _members.begin(); it != _members.end(); ++it)
	{
		IClient* member = *it;

		if (member == exclude)
			continue;
		member->getBuffer().appendWrite(message);
	}
}

std::string Channel::getMemberList() const
{
	std::string list = "";
	std::set< IClient* >::iterator it;

	for (it = _members.begin(); it != _members.end(); ++it)
	{
		IClient* member = *it;

		if (it != _members.begin())
			list += " ";

		if (isOperator(member))
			list += "@";

		list += member->getNickname();
	}
	return list;
}

bool Channel::isEmpty() const
{
	return _members.empty();
}

void Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}
void Channel::setKey(const std::string& key)
{
	_key = key;
}
void Channel::setInviteOnly(bool inviteOnly)
{
	_inviteOnly = inviteOnly;
}
void Channel::setTopicRestricted(bool restricted)
{
	_topicRestricted = restricted;
}
void Channel::setUserLimit(int limit)
{
	_userLimit = limit;
}

const std::string& Channel::getName() const
{
	return _name;
}
const std::string& Channel::getTopic() const
{
	return _topic;
}
const std::string& Channel::getKey() const
{
	return _key;
}
size_t Channel::getMemberCount() const
{
	return _members.size();
}
bool Channel::isInviteOnly() const
{
	return _inviteOnly;
}
bool Channel::isTopicRestricted() const
{
	return _topicRestricted;
}
int Channel::getUserLimit() const
{
	return _userLimit;
}

