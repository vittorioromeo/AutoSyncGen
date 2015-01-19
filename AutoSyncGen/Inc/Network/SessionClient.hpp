#ifndef AUTOSYNCGEN_SESSIONCLIENT
#define AUTOSYNCGEN_SESSIONCLIENT

namespace syn
{
	template<typename TSettings> class SessionClient : public Internal::SessionClientBase<TSettings>
	{
		friend Internal::SessionClientBase<TSettings>;

		public:
			using BaseType = Internal::SessionClientBase<TSettings>;
			using SPT = typename BaseType::SPT;
			using RPT = typename BaseType::RPT;

		private:
			IpAddress serverIp;
			Port serverPort;

			// Assigned from server after connection is accepted
			CID cid{nullCID};

			template<SPT TType, typename... TArgs> inline void sendToServerNoID(TArgs&&... mArgs)
			{
				SSVU_ASSERT(cid == nullCID);

				this->template mkPacket<TType>(ssvu::fwd<TArgs>(mArgs)...);
				this->sendTo(serverIp, serverPort);
			}	

			template<SPT TType, typename... TArgs> inline void sendToServer(TArgs&&... mArgs)
			{
				SSVU_ASSERT(cid != nullCID);

				this->template mkPacket<TType>(cid, ssvu::fwd<TArgs>(mArgs)...);
				this->sendTo(serverIp, serverPort);
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
					case RPT::SyncRequestSatisfy:
						handleSyncRequestSatisfy();
						return;					
					case RPT::SyncRequestUnneeded:
						handleSyncRequestUnneeded();
						return;					
					case RPT::SyncRequestDecline:
						handleSyncRequestDecline();
						return;					
				}
			}

			inline void sendConnectionRequest()
			{
				sendToServerNoID<SPT::ConnectionRequest>();
			}

			inline void sendPing()
			{
				this->debugLo() << "Sending ping\n";
				sendToServer<SPT::Ping>();
			}

			inline void handleConnectionAccept()
			{
				auto id(this->template popRecv<CID>());
				auto msg(this->template popRecv<std::string>()); 

				this->debugLo() 	<< "Server accepted connection - my CID is " << id << "\n"
							<< "Server message: " << msg << "\n";

				cid = id;
			}

			inline void handleConnectionDecline()
			{

			}

			inline void handleSyncRequestSatisfy()
			{

			}

			inline void handleSyncRequestUnneeded()
			{

			}

			inline void handleSyncRequestDecline()
			{

			}
			
		public:
			inline SessionClient(syn::Port mPort, syn::IpAddress mServerIp, syn::Port mServerPort) 
				: Internal::SessionClientBase<TSettings>{"Client", mPort},
				serverIp{mServerIp}, serverPort{mServerPort}
			{
				this->setBusy(true);

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
