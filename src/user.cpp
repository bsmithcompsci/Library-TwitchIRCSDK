#include "user.h"

namespace TwitchIRC
{
	User::User()
	{
		username = "[<Unknown>]";
	}
	User::User(std::string _username)
	{
		valid = true;
		username = _username;
	}
	User::~User()
	{
	}

	std::string User::GetUsername()
	{
		return username;
	}

	bool User::IsValid()
	{
		return valid;
	}

	std::string User::GetMention()
	{
		std::string stringBuilder = "@";
		stringBuilder += username;
		return stringBuilder;
	}

	void User::Timeout(const uint32_t &_time)
	{
	}

	void User::PromoteToModerator()
	{
	}

	void User::DemoteFromModerator()
	{
	}

	bool User::IsBroadcaster()
	{
		return ((int)tags & (int)UserTags::Broadcaster) != 0;
	}
	bool User::IsModerator()
	{
		return ((int)tags & (int)UserTags::Moderator) != 0;
	}
	bool User::IsTwitchModerator()
	{
		return ((int)tags & (int)UserTags::TwitchModerator) != 0;
	}
	bool User::IsSubscriber()
	{
		return ((int)tags & (int)UserTags::Subscriber) != 0;
	}
	bool User::IsPrime()
	{
		return ((int)tags & (int)UserTags::Prime) != 0;
	}
}; // namespace TwitchIRC