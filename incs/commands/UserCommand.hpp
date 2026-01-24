#pragma once

#include "ACommand.hpp"

class UserCommand : public ACommand
{
private:
	UserCommand(IServer& server);

	void doExecute(IClient* client, const Message& message);

public:
	virtual ~UserCommand();

	bool requiresRegistration() const
	{
		return false;
	}

	std::size_t minParams() const
	{
		return 4;
	}

	const char* getName() const
	{
		return "USER";
	}

	static ACommand* create(IServer& server);
};
