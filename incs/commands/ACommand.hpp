#pragma once

#include "core/IClient.hpp"
#include "core/IServer.hpp"
#include "protocol/Message.hpp"
#include "protocol/NumericReply.hpp"
#include <string>

class ACommand
{
private:
	ACommand(const ACommand& source);
	ACommand& operator=(const ACommand& other);

protected:
	IServer& m_server;

	void sendReply(IClient* client, const NumericReply& reply);
	bool validateParamCount(IClient* client, const Message& message, size_t min);

	ACommand(IServer& server);

public:
	virtual ~ACommand();

	virtual void execute(IClient* client, const Message& message) = 0;

	virtual bool requiresRegistration() const
	{
		return true;
	};

	virtual bool requiresAuthentication() const
	{
		return true;
	};

	virtual std::size_t minParams() const
	{
		return 0;
	};

	virtual std::string getName() const = 0;
};
