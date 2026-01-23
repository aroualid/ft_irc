#include "modes/KeyMode.hpp"

bool KeyMode::apply(IChannel* channel, bool set, const std::string& param, IClient* setter)
{
	(void)setter;

	if (set)
	{
		if (validateParam(param))
			channel->setKey(param);
		else
			return false;
	}
	else if (!set)
		channel->setKey("");
	return true;
}

bool KeyMode::validateParam(const std::string& param) const
{
	for (size_t i = 0; i < param.length(); i++)
	{
		if (!isalnum(param[i]))
			return false;
	}
	return true;
}
