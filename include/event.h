#pragma once
#include "user.h"
#include <string>

namespace TwitchIRC
{
	class IRCEvent
	{
		User sender;
		std::string eventName;
		std::string data;
	public:
		static IRCEvent IRCEvent::ConstructEvent(const char *_rawMessage);

		IRCEvent(User _sender, std::string _eventName, std::string _data = "");

		User GetSender();
		std::string GetEventName();
		std::string GetData();
	};
}; // namespace TwitchIRC