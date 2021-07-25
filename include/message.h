#pragma once
#include "user.h"
#include "event.h"

namespace TwitchIRC
{
	class Message
	{
		User author;
		std::string channelID;
		std::string message;
	public:
		static Message Message::ConstructMessage(IRCEvent _event);

		Message();
		Message(const User &_user, std::string _channelID, std::string _message);
		~Message();

		User GetAuthor() const;
		std::string GetChannelID() const;
		std::string GetString() const;
	};
}; // namespace TwitchIRC