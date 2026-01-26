#pragma once

#include "commands/ACommand.hpp"
#include <cstddef>

class ModeCommand : public ACommand
{
private:
	ModeCommand(IServer& server);
	void doExecute(IClient* client, const Message& message);

	static bool modeRequiresParamOnSet(char mode);
	static bool modeRequiresParamOnUnset(char mode);

public:
	virtual ~ModeCommand();
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
		return "MODE";
	}
};
