#include "twitchirc.h"
#include <filesystem>
#include <fstream>
#include <string>

namespace TwitchIRC
{
	TwitchIRC *TwitchIRC::LoadFromFile(const char *_filePath)
	{
		std::ifstream fHandle(_filePath);
		
		if (!fHandle.good())
		{
			return nullptr;
		}

		// Ensure that we can open this file.
		if (fHandle.is_open())
		{
			std::string oauthToken;
			std::string username;
			std::getline(fHandle, oauthToken); // Get the first line.
			std::getline(fHandle, username); // Get the first line.

			// If the line is empty, we know that we already failed.
			if (oauthToken.empty())
			{
				return nullptr;
			}
			if (username.empty())
			{
				return nullptr;
			}

			// Line gave us something, so we can try using it.
			return new TwitchIRC(oauthToken, username);
		}

		return nullptr;
	}

	TwitchIRC::TwitchIRC(const std::string &_oauthToken, const std::string &_username)
	{
		authToken = _oauthToken;
		username = _username;

		clientIRC.SetClientIRCRecvMessageCallback(std::bind(&TwitchIRC::HandleClientIRCMessages, this, std::placeholders::_1));
	}

	TwitchIRC::~TwitchIRC()
	{
	}

	void TwitchIRC::Poll()
	{
		clientIRC.Poll();
	}

	bool TwitchIRC::IsConnected()
	{
		return clientIRC.GetState() == ClientIRCState::Connected || clientIRC.GetState() == ClientIRCState::Connecting;
	}

	void TwitchIRC::Connect(const char *_targetChannelID)
	{
		clientIRC.Connect("irc.chat.twitch.tv", 6667, authToken.c_str(), username.c_str(), _targetChannelID);
	}

	void TwitchIRC::Disconnect()
	{
		clientIRC.Disconnect();
	}

	void TwitchIRC::SendMessage(const char *_message, const char * _channelID)
	{
		std::string data = "PRIVMSG #";
		data += _channelID;
		data += " :";
		data += _message;
		data += "\r\n";
		clientIRC.InternalSendPacket(data.c_str(), data.size());
	}

	void TwitchIRC::SetClientIRCConnectionCallback(ClientIRCConnectionCallback _callback)
	{
		clientIRC.SetClientIRCConnectionCallback(_callback);
	}

	void TwitchIRC::SetClientIRCErrorCallback(ClientIRCRecvErrorCallback _callback)
	{
		clientIRC.SetClientIRCRecvErrorCallback(_callback);
	}

	void TwitchIRC::SetReceiveMessageCallback(TwitchRecvMessageCallback _callback)
	{
		RecvMessageCallback = _callback;
	}
	void TwitchIRC::SetTwitchUserJoinedCallback(TwitchUserJoinedCallback _callback)
	{
		UserJoinedCallback = _callback;
	}
	void TwitchIRC::SetTwitchUserLeftCallback(TwitchUserLeftCallback _callback)
	{
		UserLeftCallback = _callback;
	}

	void TwitchIRC::SetTwitchUnhandledEventCallback(TwitchUnhandledEventCallback _callback)
	{
		UnhandledEventCallback = _callback;
	}

	void TwitchIRC::HandleClientIRCMessages(std::string _message)
	{
		IRCEvent ircEvent = IRCEvent::ConstructEvent(_message.c_str());

		if (ircEvent.GetEventName() == "PRIVMSG")
		{
			Message message = Message::ConstructMessage(ircEvent);

			if (message.GetAuthor().IsValid())
			{
				if (RecvMessageCallback != nullptr)
					RecvMessageCallback(message);
			}
		}
		else if (ircEvent.GetEventName() == "JOIN")
		{
			if (UserJoinedCallback != nullptr)
			{
				User tempUser = User(ircEvent.GetSender());
				UserJoinedCallback(tempUser);
			}
		}
		else if (ircEvent.GetEventName() == "PART")
		{
			if (UserJoinedCallback != nullptr)
			{
				User tempUser = User(ircEvent.GetSender());
				UserLeftCallback(tempUser);
			}
		}
		else
		{
			if (UnhandledEventCallback != nullptr)
			{
				UnhandledEventCallback(_message, ircEvent);
			}
		}
	}

}; // namespace TwitchIRC