#pragma once

#include "ACommand.hpp"

class JoinCommand : public ACommand
{
public:
	JoinCommand(Server& server);
	virtual ~JoinCommand();

	void execute(Client* client, const Message& message);

	bool requiresRegistration() const
	{
		return true;
	}
	bool requiresAuthentication() const
	{
		return true;
	}
	std::size_t minParams() const
	{
		return 1;
	}
	std::string getName() const
	{
		return "JOIN";
	}
};
