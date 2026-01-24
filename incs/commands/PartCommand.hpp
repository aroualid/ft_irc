#pragma once

#include "ACommand.hpp"
#include "IClient.hpp"
#include "IServer.hpp"
#include "protocol/Message.hpp"
#include <cstddef>

class PartCommand : public ACommand
{
private:
	PartCommand(IServer& server);

	void doExecute(IClient* client, const Message& message);

public:
	virtual ~PartCommand();

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
		return "PART";
	}

	static ACommand* create(IServer& server);
};
