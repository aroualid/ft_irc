#pragma once

#include "IServer.hpp"
#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include <cstddef>

class PassCommand : public ACommand
{
private:
	PassCommand(IServer& server);

	void doExecute(IClient* client, const Message& message);

public:
	virtual ~PassCommand();

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
		return "PASS";
	}

	static ACommand* create(IServer& server);
};
