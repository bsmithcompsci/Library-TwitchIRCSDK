#include "event.h"

namespace TwitchIRC
{

	IRCEvent IRCEvent::ConstructEvent(const char *_rawMessage)
	{
		std::string temp = _rawMessage;
		User user;

		size_t lastSemiColon = temp.find_last_of(':');
		size_t usernameEndex = temp.find_first_of('!');
		
		// This section addresses:
		// :<user>!<user>@<user>.tmi.twitch.tv JOIN #<channel>
		// :<user>!<user>@<user>.tmi.twitch.tv PART #<channel>
		// :<user>!<user>@<user>.tmi.twitch.tv PRIVMSG #<channel> :This is a sample message
		if (usernameEndex < temp.find_first_of(' ') && usernameEndex != std::string::npos)
		{
			std::string username = temp.substr(1, usernameEndex - 1);

			size_t tmiUserEndex = temp.find_first_of(' ', usernameEndex + 1);
			std::string tmiUser = temp.substr(usernameEndex + 1, tmiUserEndex - (usernameEndex + 1));

			size_t eventTypeEndex = temp.find_first_of(' ', tmiUserEndex + 1);
			std::string eventType = temp.substr(tmiUserEndex + 1, eventTypeEndex - (tmiUserEndex + 1));

			std::string data;
			if (eventTypeEndex != std::string::npos)
			{
				data = temp.substr(eventTypeEndex + 1);
			}

			user = User(username.c_str());

			return IRCEvent(user, eventType, data);
		}

		// This section addresses:
		// :tmi.twitch.tv 421 <user> WHO :Unknown command
		// PING :tmi.twitch.tv


		return IRCEvent(user, "UNKNOWN", temp);
	}

	IRCEvent::IRCEvent(User _sender, std::string _eventName, std::string _data)
	{
		sender = _sender;
		eventName = _eventName;
		data = _data;
	}
	User IRCEvent::GetSender()
	{
		return sender;
	}
	std::string IRCEvent::GetEventName()
	{
		return eventName;
	}
	std::string IRCEvent::GetData()
	{
		return data;
	}
}; // namespace TwitchIRC