#include "message.h"

namespace TwitchIRC
{
	Message Message::ConstructMessage(IRCEvent _event)
	{
		if (_event.GetEventName() != "PRIVMSG")
		{
			return Message();
		}

		std::string data = _event.GetData();

		size_t channelIDEndex = data.find_first_of(' ');
		std::string channelID = data.substr(1, channelIDEndex - 1);

		size_t messageStart = data.find_first_of(':');
		std::string message = data.substr(messageStart + 1);

		return Message(_event.GetSender(), channelID, message);
	}

	Message::Message()
	{
	}

	Message::Message(const User &_user, std::string _channelID, std::string _message)
	{
		author = _user;
		channelID = _channelID;
		message = _message;
	}

	Message::~Message()
	{
	}
	
	User Message::GetAuthor() const
	{
		return author;
	}
	std::string Message::GetChannelID() const
	{
		return channelID;
	}
	std::string Message::GetString() const
	{
		return message;
	}
}; // namespace TwitchIRC

