#ifndef AUTOSYNCGEN_SESSIONCLIENT
#define AUTOSYNCGEN_SESSIONCLIENT

namespace syn
{
	template<typename TSettings> class SessionClient : public Impl::SessionClientBase<TSettings>
	{
		friend Impl::SessionClientBase<TSettings>;

		public:
			using BaseType = Impl::SessionClientBase<TSettings>;
			using SPT = typename BaseType::SPT;
			using RPT = typename BaseType::RPT;
			using Diff = typename BaseType::Diff;

		private:
			IpAddress serverIp;
			Port serverPort;

			// Assigned from server after connection is accepted
			CID cid{nullCID};

			template<SPT TType, typename... TArgs> inline void sendToServerNoCID(TArgs&&... mArgs)
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
					case RPT::Data:
						this->onDataReceived(this->recvBuffer);
						return;
				}
			}

			inline void sendConnectionRequest()
			{
				sendToServerNoCID<SPT::ConnectionRequest>();
			}

			inline void sendPing()
			{
				this->debugLo() << "Sending ping\n";
				sendToServer<SPT::Ping>();
			}

			inline void sendSyncRequest()
			{
				this->debugLo() << "Sending sync request\n";
				sendToServer<SPT::SyncRequest>(this->syncManager.getSnapshot());
			}

			inline void handleConnectionAccept()
			{
				auto id(this->template popRecv<CID>());
				auto msg(this->template popRecv<std::string>());

				this->debugLo() << "Server accepted connection - my CID is " << id << "\n"
								<< "Server message: " << msg << "\n";

				cid = id;
			}

			inline void handleConnectionDecline()
			{

			}

			inline void handleSyncRequestSatisfy()
			{
				auto diff(this->template popRecv<Diff>());
				this->debugLo() << "Received diff \n"
								<< diff.toJson() << "\n";

				this->syncManager.applyDiff(diff);

				this->debugLo() << "Diff applied\n";
			}

			inline void handleSyncRequestUnneeded()
			{

			}

			inline void handleSyncRequestDecline()
			{

			}

		public:
			ssvu::Delegate<void(sf::Packet&)> onDataReceived;

			inline SessionClient(syn::Port mPort, syn::IpAddress mServerIp, syn::Port mServerPort)
				: Impl::SessionClientBase<TSettings>{"Client", mPort},
				serverIp{mServerIp}, serverPort{mServerPort}
			{
				this->setBusy(true);

				// TODO: wat
				auto xd = std::thread([this]
				{
					while(true)
					{
						sendConnectionRequest();
						std::this_thread::sleep_for(std::chrono::seconds(1));

						int i = 0;
						while(true)
						{
							sendPing();

							++i;

							if(i == 4)
							{
								i = 0;
								sendSyncRequest();
							}

							std::this_thread::sleep_for(std::chrono::seconds(1));
						}
					}
				});

				xd.detach();
			}

			template<typename... TArgs> inline void sendDataToServer(TArgs&&... mArgs)
			{
				sendToServer<SPT::Data>(ssvu::fwd<TArgs>(mArgs)...);
			}
	};
}

#endif
