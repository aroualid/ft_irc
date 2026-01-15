#pragma once

#include "IServer.hpp"
#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include <cstddef>

class PassCommand : public ACommand
{
private:
	PassCommand(IServer& server);

public:
	virtual ~PassCommand();

	void execute(IClient* client, const Message& message);

	bool requiresRegistration() const
	{
		return false;
	}
	bool requiresAuthentication() const
	{
		return false;
	}
	std::size_t minParams() const
	{
		return 1;
	}
	std::string getName() const
	{
		return "PASS";
	}

	static ACommand* create(IServer& server);
};
