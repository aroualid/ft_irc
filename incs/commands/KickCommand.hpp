#pragma once

#include "commands/ACommand.hpp"
#include <cstddef>

class KickCommand : public ACommand
{
private:
	KickCommand(IServer& server);
	void doExecute(IClient* client, const Message& message);

public:
	virtual ~KickCommand();
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
		return "KICK";
	}
};
