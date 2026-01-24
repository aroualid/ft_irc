#pragma once

#include "commands/ACommand.hpp"

class PingCommand : public ACommand
{
private:
	PingCommand(IServer& server);

	void doExecute(IClient* client, const Message& message);

public:
	virtual ~PingCommand();

	void execute(IClient* client, const Message& message);

	bool requiresRegistration() const
	{
		return false;
	}

	std::size_t minParams() const
	{
		return 1;
	}

	const char* getName() const
	{
		return "PING";
	}

	static ACommand* create(IServer& server);
};
