#ifndef AUTOSYNCGEN_SESSIONHOST
#define AUTOSYNCGEN_SESSIONHOST

namespace syn
{
	class SessionServer;
	class SessionClient;

	constexpr int nullClientID{-1};

	namespace Internal
	{
		template<typename TSPT, typename TRPT, typename TDerived> class SessionHost
		{
			public:	
				using SPT = TSPT;
				using RPT = TRPT;

			private:
				std::string name;
				syn::IpAddress ip;
				syn::Port port;
				sf::UdpSocket socket;	
				bool busy{false};

				inline auto& getTD() noexcept
				{
					return reinterpret_cast<TDerived&>(*this);
				}
				inline const auto& getTD() const noexcept
				{
					return reinterpret_cast<const TDerived&>(*this);
				}

				inline auto& getHandler() noexcept
				{
					return getTD().handler;
				}

				inline void tryBindSocket()
				{
					if(socket.bind(port) != sf::Socket::Done)
					{
						throw std::runtime_error("Error binding socket");
					}
					else
					{
						debugLo() << "Socket successfully bound to port " + ssvu::toStr(port) + "\n";
					}
				}

				inline void tryForwardReceivedPacket()
				{
					PacketType type;

					try
					{
						recvBuffer >> type;
						getTD().handle(static_cast<RPT>(type));
					}
					catch(std::exception& mEx)
					{
						debugLo() << "Exception during packet handling: (" << type << ")\n" << mEx.what() << "\n";
					}
					catch(...)
					{
						debugLo() << "Unknown exception during packet handling: (" << type << ")\n";
					}
				}

				inline void receiveThread()
				{
					while(true)
					{
						recvBuffer.clear();
						if(socket.receive(recvBuffer, senderIp, senderPort) != sf::Socket::Done)
						{
							//debugLo() << "Error receiving packet\n";
						}
						else
						{
							debugLo() 	<< "Packet successfully received from: \n" 
										<< "	" << senderIp << ":" << senderPort << "\n";

							tryForwardReceivedPacket();
						}
					}
				}

			protected:
				sf::Packet sendBuffer, recvBuffer;
				std::future<void> hostFuture;
				syn::IpAddress senderIp;
				syn::Port senderPort;

				inline void sendTo(const syn::IpAddress& mIp, const syn::Port& mPort)
				{
					if(socket.send(sendBuffer, mIp, mPort) != sf::Socket::Done)
					{
						debugLo() << "Error sending packet to server\n";
					}
					else
					{
						debugLo() << "Successfully sent packet to server\n";
					}
				}	

				template<SPT TType, typename... TArgs> inline void mkPacket(TArgs&&... mArgs)
				{
					sendBuffer.clear();
					sendBuffer << static_cast<PacketType>(TType);
					fillPacket(sendBuffer, ssvu::fwd<TArgs>(mArgs)...);
				}	

				inline void setBusy(bool mBusy) noexcept { busy = mBusy; }

				template<typename T> inline auto popRecv() 
				{
					T temp;
					recvBuffer >> temp;
					return temp;
				}

			public:
				inline SessionHost(std::string mName, syn::Port mPort) : name{mName}, ip{syn::IpAddress::getLocalAddress()}, port{mPort}
				{
					socket.setBlocking(true);
					tryBindSocket();
					hostFuture = std::async(std::launch::async, [this]{ receiveThread(); });
				}

				inline const auto& getName() const noexcept { return name; }
				inline const auto& getIp() const noexcept { return ip; }
				inline const auto& getPort() const noexcept { return port; }

				inline const auto& isBusy() const noexcept { return busy; }

				inline auto debugLo() -> decltype(ssvu::lo(name))
				{
					// TODO: ssvu::getNullLogStream(); for fake logs
					return ssvu::lo(name);
				}
		};

		using SessionServerBase = SessionHost<PacketStoC, PacketCtoS, SessionServer>;
		using SessionClientBase = SessionHost<PacketCtoS, PacketStoC, SessionClient>;
	}	
}

#endif
