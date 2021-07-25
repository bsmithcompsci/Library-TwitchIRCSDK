#pragma once
#include "user.h"
#include "message.h"
#include "clientIRC.h"
#include <functional>

namespace TwitchIRC
{
    typedef std::function<void(const Message &_message)> TwitchRecvMessageCallback;
    typedef std::function<void(const User &_user)> TwitchUserJoinedCallback;
    typedef std::function<void(const User &_user)> TwitchUserLeftCallback;
    typedef std::function<void(std::string _rawMessage, IRCEvent _event)> TwitchUnhandledEventCallback;

    class TwitchIRC
    {
        std::string authToken;
        std::string username;

        ClientIRC clientIRC;

        TwitchRecvMessageCallback RecvMessageCallback;
        TwitchUserJoinedCallback UserJoinedCallback;
        TwitchUserLeftCallback UserLeftCallback;

        TwitchUnhandledEventCallback UnhandledEventCallback;

        void HandleClientIRCMessages(std::string _message);
    public:
        static TwitchIRC *LoadFromFile(const char *_filePath);

        TwitchIRC(const std::string &_oauthToken, const std::string &_username);
        ~TwitchIRC();

        void Poll();

        bool IsConnected();

        void Connect(const char *_targetChannelID);
        void Disconnect();

        void TwitchIRC::SendMessage(const char *_message, const char *_channelID);

        void SetClientIRCConnectionCallback(ClientIRCConnectionCallback _callback);
        void SetClientIRCErrorCallback(ClientIRCRecvErrorCallback _callback);
        void SetReceiveMessageCallback(TwitchRecvMessageCallback _callback);

        void SetTwitchUnhandledEventCallback(TwitchUnhandledEventCallback _callback);

        void SetTwitchUserJoinedCallback(TwitchUserJoinedCallback _callback);
        void SetTwitchUserLeftCallback(TwitchUserLeftCallback _callback);
    };
}; // namespace TwitchIRC
