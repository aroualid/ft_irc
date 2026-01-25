#pragma once

#include "ACommand.hpp"

class NickCommand : public ACommand
{
private:
	NickCommand(IServer& server);

	void doExecute(IClient* client, const Message& message);

public:
	virtual ~NickCommand();

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
		return "NICK";
	}

	static ACommand* create(IServer& server);
};
