#pragma once

#include "ACommand.hpp"

class UserCommand : ACommand
{
public:
	UserCommand(Server* server);
	virtual ~UserCommand();

	void execute(Client* client, const Message& message);

	bool requiresRegistration() const
	{
		return false;
	}
	bool requiresAuthentication() const
	{
		return true;
	}
	std::size_t minParams() const
	{
		return 4;
	}
	std::string getName() const
	{
		return "USER";
	}
};
