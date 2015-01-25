#ifndef AUTOSYNCGEN_SESSIONHOST
#define AUTOSYNCGEN_SESSIONHOST

namespace syn
{
	template<typename> class SessionServer;
	template<typename> class SessionClient;

	namespace Impl
	{
		template<typename TSettings, typename TSPT, typename TRPT, template<typename> class TDerivedBase> class SessionHost
		{
			public:
				using SPT = TSPT;
				using RPT = TRPT;
				using Settings = TSettings;
				using Derived = TDerivedBase<Settings>;
				using SyncManager = typename Settings::SyncManager;
				using Diff = typename SyncManager::DiffType;
				using Snapshot = typename SyncManager::SnapshotType;

			private:
				std::string name;
				IpAddress ip;
				Port port;
				UdpSocket socket;
				bool busy{false};

				inline auto& getTD() noexcept
				{
					return reinterpret_cast<Derived&>(*this);
				}
				inline const auto& getTD() const noexcept
				{
					return reinterpret_cast<const Derived&>(*this);
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
					PT::NType type;

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
				Packet sendBuffer, recvBuffer;
				std::future<void> hostFuture;
				IpAddress senderIp;
				Port senderPort;
				SyncManager syncManager;

				inline void sendTo(const IpAddress& mIp, const Port& mPort)
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
					sendBuffer << static_cast<PT::NType>(TType);
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
				inline SessionHost(std::string mName, syn::Port mPort) : name{mName}, ip{IpAddress::getLocalAddress()}, port{mPort}
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
					// return ssvu::loNull(name);
					return ssvu::lo(name);
				}

				inline auto& getSyncManager() noexcept { return syncManager; }
		};

		template<typename TSettings> using SessionServerBase = SessionHost<TSettings, PT::StoC, PT::CtoS, SessionServer>;
		template<typename TSettings> using SessionClientBase = SessionHost<TSettings, PT::CtoS, PT::StoC, SessionClient>;
	}
}

#endif
