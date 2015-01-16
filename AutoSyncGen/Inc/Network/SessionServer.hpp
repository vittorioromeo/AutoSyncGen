#ifndef AUTOSYNCGEN_SESSIONSERVER
#define AUTOSYNCGEN_SESSIONSERVER

namespace syn
{
	using ClientID = int;

	namespace Internal
	{
		constexpr int maxSecondsUntilTimeout{5};

		struct ConnectionData
		{
			ClientID id;
			syn::IpAddress ip;
			syn::Port port;
			int secondsUntilTimeout;
		};

		class ConnectionManager
		{
			private:
				ClientID nextID{0};
				std::map<ClientID, ConnectionData> connections;
				std::future<void> timeoutThread;
				bool busy{true};

			public:
				inline ConnectionManager()
				{
					timeoutThread = std::async(std::launch::async, [this]
					{
						std::vector<ClientID> toDisconnect;

						while(busy)
						{
							toDisconnect.clear();	

							for(auto& p : connections)
							{
								auto& c(p.second);

								--(c.secondsUntilTimeout);
								if(c.secondsUntilTimeout == 0)
								{
									toDisconnect.emplace_back(c.id);
								}
							}

							for(auto id : toDisconnect)
							{
								forceDisconnect(id);
							}

							std::this_thread::sleep_for(std::chrono::seconds(1));
						}
					});
				}

				inline auto& create(const syn::IpAddress& mIp, const syn::Port& mPort)
				{
					auto idToUse(nextID++);
					connections[idToUse] = ConnectionData{idToUse, mIp, mPort, maxSecondsUntilTimeout};
					return connections[idToUse];
				}

				inline void forceDisconnect(ClientID mID)
				{
					SSVU_ASSERT(has(mID));

					connections.erase(mID);

					ssvu::lo("TODO") << "Client " << mID << " was disconnected\n";
				}

				inline bool has(ClientID mID) const noexcept
				{
					return connections.find(mID) != std::end(connections); 
				}

				inline auto& operator[](ClientID mID) noexcept
				{
					SSVU_ASSERT(has(mID));
					return connections[mID];
				}

				inline void pingReceived(ClientID mID) noexcept
				{
					SSVU_ASSERT(has(mID));
					connections[mID].secondsUntilTimeout = maxSecondsUntilTimeout;
				}	
		};	
	}

	class SessionServer : public Internal::SessionServerBase
	{
		friend Internal::SessionServerBase;


		public:
				

		private:
			Internal::ConnectionManager cManager;

			template<SPT TType, typename... TArgs> inline void sendToClient(ClientID mID, TArgs&&... mArgs)
			{
				if(!cManager.has(mID))
				{
					debugLo() << "Wanted to send a packet to CID " << mID << ", but it's not connected\n";
					return;
				}

				mkPacket<TType>(ssvu::fwd<TArgs>(mArgs)...);


				const auto& connection(cManager[mID]);
				sendTo(connection.ip, connection.port);
			}	

			inline void handle(RPT mType)
			{
				if(mType == RPT::ConnectionRequest)
				{
					handleConnectionRequest();
					return;
				}

				auto cid(popRecv<ClientID>());

				if(!cManager.has(cid))
				{
					debugLo() << "Received a packet from non-connected CID " << cid << "\n";
					return;
				}

				switch(mType)
				{						
					case RPT::Ping:
						handlePing(cid);
						return;
				}
			}

			inline void handleConnectionRequest()
			{
				debugLo() << "Connection request received\n";
				debugLo() << "Accepting request\n";

				auto cid(cManager.create(senderIp, senderPort).id);
				sendConnectionAccept(cid);
			}

			inline void handlePing(ClientID mID)
			{
				cManager.pingReceived(mID);
			}

			inline void sendConnectionAccept(ClientID mID)
			{
				sendToClient<SPT::ConnectionAccept>(mID, mID, "Welcome to the server.");
			}

		public:
			inline SessionServer(syn::Port mPort) : Internal::SessionServerBase{"Server", mPort}
			{
				setBusy(true);
			}
	};
}

#endif
