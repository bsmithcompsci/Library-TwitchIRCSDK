#pragma once
#include <cstdint>
#include <string>

namespace TwitchIRC
{
	enum class UserTags : uint16_t
	{
		Viewer				= 0,		
		Broadcaster			= 1,		
		Moderator			= 1 << 1,	
		Subscriber			= 1 << 2,	
		Prime				= 1 << 3,	
		TwitchModerator		= 1 << 4	
	};

	class User
	{
		std::string username;
		UserTags tags = UserTags::Viewer;
		bool valid = false;
	public:
		User();
		User(std::string _username);
		~User();

		bool IsValid();
		std::string GetUsername();
		std::string GetMention();					// Gets the mention format to append to your message.

		void Timeout(const uint32_t &_time = 0);	// Timeout this user either for a temporary time or forever. (0 for forever!)
		void PromoteToModerator();					// Promotes this user to moderator.
		void DemoteFromModerator();					// Demotes this user from moderator.

		bool IsBroadcaster();						// Check, if this user is the broadcasting user.
		bool IsModerator();							// Check, if this user is a moderator for the broadcasting user.
		bool IsTwitchModerator();					// Check, if this user is a moderator from Twitch.
		bool IsSubscriber();						// Check, if this user is a Subscriber of the channel.
		//bool IsFollower();						// Check, if this user is a follower of the channel. [Might not be possible, if you're external - not the broadcaster.]
		bool IsPrime();								// Check, if this user is a prime user.
		//bool IsTimedout();						// Check, if this user is timedout.	[Might not be possible, unless you are a moderator...]
	};

}; // namespace TwitchIRC