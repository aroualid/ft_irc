#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel(const std::string& name) : _name(name), _topic(""), _key(""), _inviteOnly(false), _topicRestricted(false), _userLimit(-1)
{
}

Channel::~Channel() {
    _members.clear();
    _operators.clear();
    _invited.clear();
}



bool Channel::addMember(Client* client, const std::string& key)
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
	return true;
}

void Channel::removeMember(Client* client)
{
	_members.erase(client);
}

bool Channel::hasMember(Client* client) const {
    return _members.find(client) != _members.end();
}

void Channel::addOperator(Client* client)
{
	_operators.insert(client);
}

bool Channel::isOperator(Client* client) const
{
	return (_operators.find(client) != _operators.end());
}

bool Channel::applyMode(char mode, bool set, const std::string& param, Client* setter)
{
	if (!isOperator(setter))
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
		}
		return false;
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
				_operators.erase(setter);
				return true;
			case 'l':
				_userLimit = -1;
				return true;
		}
		return false;
	}
	return false;
}

std::string Channel::getModeString() const
{
	std::string modes = "+";
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
	//if (_userLimit)
		//modes += " " + std::to_string(_userLimit); CA C EST DU CPP11 pas 98

	return modes;

}

void Channel::broadcast(const std::string& message, Client* exclude) {
    std::set<Client*>::iterator it;
    
    for (it = _members.begin(); it != _members.end(); ++it) {
        Client* member = *it;
        
        // On n'envoie pas le message à celui qui l'a écrit (si exclude est fourni)
        if (member == exclude)
            continue;
            
        // Envoi basique via socket. 
        // Note: Assure-toi que le message termine bien par \r\n
        send(member->getFd(), message.c_str(), message.length(), 0);
    }
}

std::string Channel::getMemberList() const {
    std::string list = "";
    std::set<Client*>::iterator it;

    for (it = _members.begin(); it != _members.end(); ++it) {
        Client* member = *it;
        
        if (it != _members.begin()) list += " ";
        
        // Préfixe pour les opérateurs (@)
        if (isOperator(member))
            list += "@";
        
        list += member->getNickname();
    }
    return list;
}
