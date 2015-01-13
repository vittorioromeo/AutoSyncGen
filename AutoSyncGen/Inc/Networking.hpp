/*#ifndef AUTOSYNCGEN_NETWORKING
#define AUTOSYNCGEN_NETWORKING

#include <SFML/System.hpp>
#include <SFML/Network.hpp>
#include "../../AutoSyncGen/Inc/Common.hpp"

namespace syn
{
	using IpAddress = sf::IpAddress;

	namespace Internal
	{
		class UdpSocketSFML
		{
			private:
				sf::UdpSocket socket;

			public:
				inline UdpSocketSFML() 
				{

				}

				inline void bind(Port mPort)
				{
					auto result(socket.bind(mPort));
					
					if(result != sf::Socket::Done)
					{
						throw std::runtime_error("Error binding socket on port " + ssvu::toStr(mPort));
					}
				}

				inline void send(IpAddress mTarget, Port mPort, const std::vector<char>& mData)
				{					
					auto result(socket.send(mData.data(), mData.size(), mTarget, mPort));

					if(result != sf::Socket::Done)
					{
						throw std::runtime_error("Error sending data from socket on ip, port " + ssvu::toStr(mTarget) + ", " + ssvu::toStr(mPort));
					}
				}

				inline auto receive(IpAddress mSender, Port mPort, ssvu::SizeT mToReceive)
				{
					std::vector<char> buffer;
					ssvu::SizeT received;

					buffer.resize(mToReceive);

					auto result(socket.receive(buffer.data(), mToReceive, received, mSender, mPort));

					if(result != sf::Socket::Done)
					{
						throw std::runtime_error("Error receiving data from socket on ip, port " + ssvu::toStr(mSender) + ", " + ssvu::toStr(mPort));
					}

					return buffer;
				}
		};
	}

	using UdpSocket = Internal::UdpSocketSFML;



}

#endif*/