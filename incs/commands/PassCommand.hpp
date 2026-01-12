#pragma once

#include "commands/ACommand.hpp"
#include <cstddef>

class PassCommand : public ACommand
{
public:
	PassCommand(IServer& server);
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
};
