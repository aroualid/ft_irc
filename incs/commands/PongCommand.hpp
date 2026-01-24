#pragma once

#include "commands/ACommand.hpp"

class PongCommand : public ACommand
{
private:
	PongCommand(IServer& server);

	void doExecute(IClient* client, const Message& message);

public:
	virtual ~PongCommand();

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
		return "PONG";
	}

	static ACommand* create(IServer& server);
};
