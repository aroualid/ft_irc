#pragma once

#include "commands/ACommand.hpp"
#include <cstddef>

class InviteCommand : public ACommand
{
private:
	InviteCommand(IServer& server);
	void doExecute(IClient* client, const Message& message);

public:
	virtual ~InviteCommand();
	static ACommand* create(IServer& server);

	bool requiresRegistration() const
	{
		return true;
	}

	std::size_t minParams() const
	{
		return 2;
	}

	const char* getName() const
	{
		return "INVITE";
	}
};
