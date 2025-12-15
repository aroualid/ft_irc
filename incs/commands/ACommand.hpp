#pragma once

#include <cstddef>
#include <string>

class Server;
class Client;
class NumericReply;
class Message;

class ACommand
{
protected:
	Server* m_server;

	void sendReply(Client* client, const NumericReply& reply);
	bool validateParamCount(Client* client, const Message& message, size_t min);

public:
	ACommand(Server* server);
	virtual ~ACommand();

	virtual void execute(Client* client, const Message& message) = 0;

	virtual bool requiresRegistration() const
	{
		return true;
	};

	virtual bool requiresAuthentication() const
	{
		return true;
	};

	virtual size_t minParams() const
	{
		return 0;
	};

	virtual std::string getName() const = 0;
};
