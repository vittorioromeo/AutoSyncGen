#ifndef AUTOSYNCGEN_SESSIONCLIENT
#define AUTOSYNCGEN_SESSIONCLIENT

namespace syn
{
	class SessionClient : public Internal::SessionClientBase
	{
		friend Internal::SessionClientBase;

		private:			
			syn::IpAddress serverIp;
			syn::Port serverPort;

			// Assigned from server after connection is accepted
			int clientID{nullClientID};

			template<SPT TType, typename... TArgs> inline void sendToServerNoID(TArgs&&... mArgs)
			{
				SSVU_ASSERT(clientID == nullClientID);

				mkPacket<TType>(ssvu::fwd<TArgs>(mArgs)...);
				sendTo(serverIp, serverPort);
			}	

			template<SPT TType, typename... TArgs> inline void sendToServer(TArgs&&... mArgs)
			{
				SSVU_ASSERT(clientID != nullClientID);

				mkPacket<TType>(clientID, ssvu::fwd<TArgs>(mArgs)...);
				sendTo(serverIp, serverPort);
			}	

			inline void handle(RPT mType)
			{
				switch(mType)
				{
					case RPT::ConnectionAccept:
						handleConnectionAccept();
						return;
					case RPT::ConnectionDecline:
						handleConnectionDecline();
						return;					
				}
			}

			inline void sendConnectionRequest()
			{
				sendToServerNoID<SPT::ConnectionRequest>();
			}

			inline void sendPing()
			{
				debugLo() << "Sending ping\n";
				sendToServer<SPT::Ping>();
			}

			inline void handleConnectionAccept()
			{
				auto id(popRecv<ClientID>());
				auto msg(popRecv<std::string>()); 

				debugLo() 	<< "Server accepted connection - my CID is " << id << "\n"
							<< "Server message: " << msg << "\n";

				clientID = id;
			}

			inline void handleConnectionDecline()
			{

			}
			
		public:
			inline SessionClient(syn::Port mPort, syn::IpAddress mServerIp, syn::Port mServerPort) 
				: Internal::SessionClientBase{"Client", mPort},
				serverIp{mServerIp}, serverPort{mServerPort}
			{
				setBusy(true);

				auto xd = std::thread([this]
				{	
					while(true)
					{
						sendConnectionRequest();
						std::this_thread::sleep_for(std::chrono::seconds(1));

						while(true)
						{
							sendPing();
							std::this_thread::sleep_for(std::chrono::seconds(1));
						}
					}
				});
				xd.detach();
			}

	};
}

#endif
