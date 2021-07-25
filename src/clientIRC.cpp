#include "clientIRC.h"
#include "asio.hpp"

#define NetworkBufferSize 1024 * 1024 * 1024

namespace TwitchIRC
{
	// Declare the forwarded structs.
	// We hide these away from the header, so it's less intensive to append everything into the headers include table.
	// Additionally, Windows like to include <winsock2.h> twice, which causes compiliation issues with larger projects that use asio.
	struct ClientIRCSocketContext
	{
		asio::io_context context;
	};

	struct ClientIRCSocket
	{
		asio::ip::tcp::socket internalSocket;
		explicit ClientIRCSocket(ClientIRCSocketContext &_context) : internalSocket(_context.context) {}
		explicit ClientIRCSocket(ClientIRCSocketContext *_context) : internalSocket((*_context).context) {}
		~ClientIRCSocket()
		{
			asio::error_code ec;
			internalSocket.cancel(ec);
			internalSocket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
			internalSocket.release(ec);
		}
	};
	struct ClientIRCSocketResolver
	{
		asio::ip::tcp::resolver internalResolver;
		explicit ClientIRCSocketResolver(ClientIRCSocketContext &_context) : internalResolver(_context.context) {}
		explicit ClientIRCSocketResolver(ClientIRCSocketContext *_context) : internalResolver((*_context).context) {}
	};
	struct ClientIRCStreamBuffer
	{
		asio::streambuf internalBuffer;
	};

	// ////////////////////////////////////////////////////////////////////////////////

	void ConnectionResolver(ClientIRCData *_data, uint16_t _port, const asio::error_code &_ec, asio::ip::tcp::resolver::results_type _results);
	void NextConnection(ClientIRCData *_data, std::vector<asio::ip::tcp::endpoint> &_endpoints);
	void ConnectHandler(ClientIRCData *_data, std::vector<asio::ip::tcp::endpoint> &_endpoints, const asio::error_code &_ec, const asio::ip::tcp::endpoint &_endpoint);

	void SetupReadFromSocket(ClientIRCData *_data);
	void ReadFromSocket(ClientIRCData *_data, const asio::error_code &_ec, size_t _length);

	ClientIRC::ClientIRC()
	{
		data.context = new ClientIRCSocketContext();
	}

	ClientIRC::~ClientIRC()
	{
		Disconnect();
		delete data.context;
	}

	ClientIRCState ClientIRC::GetState()
	{
		return data.state;
	}

	void ClientIRC::Poll()
	{
		// If our network stopped for any reason, we want to reboot it; and remain connected.
		if (data.context->context.stopped())
		{
			data.context->context.reset();
		}

		// Non-blocking poll from the network calls.
		data.context->context.poll();

		if (data.state == ClientIRCState::Connected && !data.socket->internalSocket.is_open())
		{
			data.state = ClientIRCState::Disconnected;
		}

		if (data.state == ClientIRCState::Disconnected)
		{
			if (data.socket != nullptr)
			{
				delete data.socket;
				data.socket = nullptr;
			}
		}
	}

	void ClientIRC::InternalSendPacket(const char *_data, size_t _dataSize)
	{
		asio::error_code ec;
		asio::write(data.socket->internalSocket, asio::buffer(_data, _dataSize), ec);
		if (ec)
		{
			if (data.ErrorCallback != nullptr)
			{
				data.ErrorCallback(ec.message());
			}
			
			data.state = ClientIRCState::Disconnected;
			return;
		}
	}

	void SetupReadFromSocket(ClientIRCData *_data)
	{
		_data->streamBuffer = new ClientIRCStreamBuffer();
		asio::async_read_until(_data->socket->internalSocket, _data->streamBuffer->internalBuffer, "\n", std::bind(&ReadFromSocket, _data, std::placeholders::_1, std::placeholders::_2));
	}

	void ReadFromSocket(ClientIRCData *_data, const asio::error_code &_ec, size_t _length)
	{
		if (_ec)
		{
			//if (_ec != asio::error::connection_reset && _ec != asio::error::eof)
			{
				if (_data->ErrorCallback != nullptr)
					_data->ErrorCallback(_ec.message());
			}
			_data->state = ClientIRCState::Disconnected;
			delete _data->streamBuffer;
			return;
		}
		
		std::string twitchMessage;

		std::istream istream(&_data->streamBuffer->internalBuffer);
		std::getline(istream, twitchMessage);
		
		// Now we can send this message to a Message Encoder.
		if (_data->RecvCallback != nullptr)
			_data->RecvCallback(twitchMessage);

		delete _data->streamBuffer;
		_data->streamBuffer = nullptr;

		SetupReadFromSocket(_data);
	}

	void ConnectHandler(ClientIRCData *_data, std::vector<asio::ip::tcp::endpoint> &_endpoints, const asio::error_code &_ec, const asio::ip::tcp::endpoint &_endpoint)
	{
		if (!_ec)
		{
			// Setup MTU and other configurations.
			{
				asio::socket_base::receive_buffer_size option(NetworkBufferSize);
				_data->socket->internalSocket.set_option(option);
			}
			{
				asio::socket_base::send_buffer_size option(NetworkBufferSize);
				_data->socket->internalSocket.set_option(option);
			}
			{
				asio::ip::tcp::no_delay option(true);
				_data->socket->internalSocket.set_option(option);
			}

			// Verify connection.
			if (!_data->socket->internalSocket.is_open())
			{
				_data->state = ClientIRCState::Disconnected;
			}

			// Authorize ourselves.
			asio::error_code ec;
			std::string sendingBuffer;
			{
				sendingBuffer = "PASS oauth:";
				sendingBuffer += _data->authToken;
				sendingBuffer += "\r\n";
				asio::write(_data->socket->internalSocket, asio::buffer(sendingBuffer.c_str(), sendingBuffer.size()), ec);
				if (ec)
				{
					_data->state = ClientIRCState::Disconnected;
				}
			}
			
			{
				sendingBuffer = "NICK ";
				sendingBuffer += _data->username;
				sendingBuffer += "\r\n";
				asio::write(_data->socket->internalSocket, asio::buffer(sendingBuffer.c_str(), sendingBuffer.size()), ec);
				if (ec)
				{
					_data->state = ClientIRCState::Disconnected;
				}
			}
			
			{
				sendingBuffer = "JOIN #";
				sendingBuffer += _data->channelID;
				sendingBuffer += "\r\n";
				asio::write(_data->socket->internalSocket, asio::buffer(sendingBuffer.c_str(), sendingBuffer.size()), ec);
				if (ec)
				{
					_data->state = ClientIRCState::Disconnected;
				}
			}

			// Ensure that there wasn't any errors in our testing above.
			if (_data->state != ClientIRCState::Disconnected)
			{
				if (_data->ConnectionCallback != nullptr)
				{
					size_t bufferLen = _endpoint.address().to_string().size() + 8;
					char *buffer = new char[bufferLen];
					snprintf(buffer, bufferLen, "%s:%i", _endpoint.address().to_string().c_str(), _endpoint.port());
					_data->ConnectionCallback(false, std::string(buffer, bufferLen));
					delete[] buffer;
				}

				_data->state = ClientIRCState::Connected;
				_endpoints.clear();

				SetupReadFromSocket(_data);
				return;
			}
		}

		delete _data->socket;
		_data->socket = nullptr;

		if (!_endpoints.empty())
		{
			NextConnection(_data, _endpoints);
		}
		else
		{
			if (_data->ConnectionCallback != nullptr)
			{
				size_t bufferLen = _endpoint.address().to_string().size() + sizeof(uint16_t) + _ec.message().size();
				char *buffer = new char[bufferLen];
				snprintf(buffer, bufferLen, "%s:%i - %s", _endpoint.address().to_string().c_str(), _endpoint.port(), _ec.message().c_str());
				_data->ConnectionCallback(true, std::string(buffer, bufferLen));
				delete[] buffer;
			}
		}
	}
	void NextConnection(ClientIRCData *_data, std::vector<asio::ip::tcp::endpoint> &_endpoints)
	{
		if (_endpoints.empty()) 
			return;

		asio::ip::tcp::endpoint endpoint = _endpoints.back();
		_endpoints.pop_back();

		_data->socket = new ClientIRCSocket(_data->context);

		_data->socket->internalSocket.async_connect(endpoint, std::bind(&ConnectHandler, _data, _endpoints, std::placeholders::_1, endpoint));
	}

	void ConnectionResolver(ClientIRCData *_data, uint16_t _port, const asio::error_code &_ec, asio::ip::tcp::resolver::results_type _results) 
	{
		if (_ec)
		{
			_data->state = ClientIRCState::Disconnected;
			if (_data->ConnectionCallback != nullptr)
				_data->ConnectionCallback(true, _ec.message());
			return;
		}

		std::vector<asio::ip::tcp::endpoint> endpoints;
		for (auto it = _results.begin(); it != _results.end(); ++it)
		{
			endpoints.push_back(asio::ip::tcp::endpoint(it->endpoint().address(), _port));
		}

		delete _data->resolver;
		_data->resolver = nullptr;

		if (!endpoints.empty())
		{
			NextConnection(_data, endpoints);
		}
	}

	void ClientIRC::Connect(const char *_hostname, uint16_t _port, const char *_authToken, const char *_username, const char * _channelID)
	{
		if (data.socket != nullptr)
		{
			// Remove the old.
			Disconnect();
		}

		data.authToken = _authToken;
		data.username = _username;
		data.channelID = _channelID;

		data.state = ClientIRCState::Connecting;

		data.resolver = new ClientIRCSocketResolver(data.context);
		asio::ip::tcp::resolver::query query(_hostname, "");

		data.resolver->internalResolver.async_resolve(query, std::bind(&ConnectionResolver, &data, _port, std::placeholders::_1, std::placeholders::_2));
	}

	void ClientIRC::Disconnect()
	{
		data.state = ClientIRCState::Disconnected;

		if (data.socket != nullptr)
		{
			delete data.socket;
			data.socket = nullptr;
		}

		if (data.context != nullptr)
		{
			data.context->context.post([&]()
			{
				if (data.resolver != nullptr)
				{
					data.resolver->internalResolver.cancel(); // Cancel the async function.
					delete data.resolver;
					data.resolver = nullptr;
				}
			});
		}
	}


	void ClientIRC::SetClientIRCConnectionCallback(ClientIRCConnectionCallback _callback)
	{
		data.ConnectionCallback = _callback;
	}
	void ClientIRC::SetClientIRCRecvMessageCallback(ClientIRCRecvMessageCallback _callback)
	{
		data.RecvCallback = _callback;
	}

	void ClientIRC::SetClientIRCRecvErrorCallback(ClientIRCRecvErrorCallback _callback)
	{
		data.ErrorCallback = _callback;
	}

}; // namespace TwitchIRC
