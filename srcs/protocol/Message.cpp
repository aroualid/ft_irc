#include "protocol/Message.hpp"

Message::Message() : m_prefix(""), m_command("")
{
}

bool Message::isValid() const
{
	return (!m_command.empty());
}

std::ostream& operator<<(std::ostream& stream, const Message& message)
{
	stream << "\tValid: " << (message.isValid() ? "yes" : "no") << "\n";
	stream << "\tPrefix: " << message.m_prefix << "\n";
	stream << "\tCommand: " << message.m_command << "\n";
	stream << "\tParameters:\n";
	if (message.m_params.empty())
		stream << "\t\t<no params supplied>";
	else
	{
		for (size_t i = 0; i < message.m_params.size(); i++)
			stream << "\t\t[" << i << "]: " << message.m_params[i]
				   << (i == message.m_params.size() - 1 ? "" : "\n");
	}
	return (stream);
}
