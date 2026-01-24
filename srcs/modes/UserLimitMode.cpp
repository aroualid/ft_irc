#include "modes/UserLimitMode.hpp"
#include "core/IChannel.hpp"
#include "modes/IChannelMode.hpp"
#include <cstdlib>
#include <sstream>
bool UserLimitMode::apply(IChannel* channel, bool set, const std::string& param, IClient* setter)
{
	(void)setter;
	if (set)
	{
		if (validateParam(param))
			channel->setUserLimit(std::atoi(param.c_str()));
		else
			return false;
	}
	else if (!set)
	{
		channel->setUserLimit(-1);
	}
	return true;
}

bool UserLimitMode::validateParam(const std::string& param) const
{
	for (size_t i = 0; i < param.length(); i++)
	{
		if (!isdigit(param[i]))
			return false;
	}
	return true;
}

std::string UserLimitMode::getParam(IChannel* channel) const
{
	std::stringstream ss;

	ss << channel->getUserLimit();

	return ss.str();
}
