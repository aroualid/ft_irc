#pragma once

#include "ACommand.hpp"

class JoinCommand : public ACommand
{
private:
	bool isValidChannelName(const std::string& name) const;

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
