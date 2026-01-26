#pragma once

#include "commands/ACommand.hpp"
#include <cstddef>

class TopicCommand : public ACommand
{
private:
	TopicCommand(IServer& server);
	void doExecute(IClient* client, const Message& message);

public:
	virtual ~TopicCommand();
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
		return "TOPIC";
	}
};
