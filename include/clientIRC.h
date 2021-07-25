#pragma once
#include "user.h"
#include <functional>
#include <string>
#include <format>

// Follows: 
// https://dev.twitch.tv/docs/irc/guide
// https://datatracker.ietf.org/doc/html/rfc1459.html

namespace TwitchIRC
{
	typedef std::function<void(const bool _error, const std::string _errorMessage)> ClientIRCConnectionCallback;
	typedef std::function<void(const std::string _errorMessage)> ClientIRCRecvErrorCallback;
	typedef std::function<void(const std::string _recvMessage)> ClientIRCRecvMessageCallback;

	enum class ClientIRCState
	{
		Initializing,
		Connecting,
		Connected,
		Disconnected
	};

	struct ClientIRCSocket;
	struct ClientIRCSocketContext;
	struct ClientIRCSocketResolver;
	struct ClientIRCStreamBuffer;

	struct ClientIRCData
	{
		ClientIRCState state = ClientIRCState::Initializing;

		ClientIRCSocketContext *context = nullptr;
		ClientIRCSocket *socket = nullptr;
		ClientIRCSocketResolver *resolver = nullptr;
		ClientIRCStreamBuffer *streamBuffer = nullptr;

		ClientIRCConnectionCallback ConnectionCallback;
		ClientIRCRecvMessageCallback RecvCallback;
		ClientIRCRecvErrorCallback ErrorCallback;

		std::string authToken;
		std::string username;
		std::string channelID;
	};

	class ClientIRC
	{
		ClientIRCData data;
	public:
		ClientIRC();
		~ClientIRC();

		ClientIRCState GetState();

		void Poll();

		void Connect(const char *_hostname, uint16_t _port, const char *_authToken, const char *_username, const char *_channelID);
		void Disconnect();

		void InternalSendPacket(const char *_data, size_t _dataSize);

		void SetClientIRCConnectionCallback(ClientIRCConnectionCallback _callback);
		void SetClientIRCRecvMessageCallback(ClientIRCRecvMessageCallback _callback);
		void SetClientIRCRecvErrorCallback(ClientIRCRecvErrorCallback _callback);
	};

}; // namespace TwitchIRC