#pragma once

#include "commands/ACommand.hpp"
#include <cstddef>

class NamesCommand : public ACommand
{
private:
	NamesCommand(IServer& server);
	void doExecute(IClient* client, const Message& message);

public:
	virtual ~NamesCommand();
	static ACommand* create(IServer& server);

	bool requiresRegistration() const
	{
		return true;
	}

	std::size_t minParams() const
	{
		return 1;
	}

	const char* getName() const
	{
		return "NAMES";
	}
};
