#include "protocol/Message.hpp"

Message::Message() : m_prefix(""), m_command("")
{
}

bool Message::isValid() const
{
	return (!m_command.empty());
}
