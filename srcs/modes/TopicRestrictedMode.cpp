#include "modes/TopicRestrictedMode.hpp"

bool TopicRestrictedMode::apply(IChannel* channel, bool set, const std::string& param,
								IClient* setter)
{
	(void)param;
	(void)setter;

	channel->setTopicRestricted(set);
	return true;
}

bool TopicRestrictedMode::validateParam(const std::string& param) const
{
	(void)param;
	return true;
}
