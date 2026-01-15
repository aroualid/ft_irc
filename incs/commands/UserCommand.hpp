#pragma once

#include "ACommand.hpp"

class UserCommand : public ACommand
{
private:
	UserCommand(IServer& server);

public:
	virtual ~UserCommand();

	void doExecute(IClient* client, const Message& message);

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

	static ACommand* create(IServer& server);
};
