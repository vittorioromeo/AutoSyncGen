#ifndef AUTOSYNCGEN_SESSIONSERVER
#define AUTOSYNCGEN_SESSIONSERVER

namespace syn
{
	template<typename TSettings> class SessionServer : public Impl::SessionServerBase<TSettings>
	{
		friend Impl::SessionServerBase<TSettings>;


		public:
			using BaseType = Impl::SessionServerBase<TSettings>;
			using SPT = typename BaseType::SPT;
			using RPT = typename BaseType::RPT;
			using Diff = typename BaseType::Diff;

		private:
			Impl::ConnectionManager cManager;

			template<SPT TType, typename... TArgs> inline void sendToClient(CID mCID, TArgs&&... mArgs)
			{
				if(!cManager.has(mCID))
				{
					this->debugLo() << "Wanted to send a packet to CID " << mCID << ", but it's not connected\n";
					return;
				}

				this->template mkPacket<TType>(ssvu::fwd<TArgs>(mArgs)...);


				const auto& connection(cManager[mCID]);
				this->sendTo(connection.ip, connection.port);
			}	

			inline void handle(RPT mType)
			{
				if(mType == RPT::ConnectionRequest)
				{
					handleConnectionRequest();
					return;
				}

				auto cid(this->template popRecv<CID>());

				if(!cManager.has(cid))
				{
					this->debugLo() << "Received a packet from non-connected CID " << cid << "\n";
					return;
				}

				switch(mType)
				{				
					case RPT::ConnectionRequest:
						throw std::runtime_error("This packet has to be handle in a special way");
					case RPT::Ping:
						handlePing(cid);
						return;
					case RPT::SyncRequest:
						handleSyncRequest(cid);
						return;
				}
			}

			inline void handleConnectionRequest()
			{
				this->debugLo() << "Connection request received\n";
				this->debugLo() << "Accepting request\n";

				auto cid(cManager.create(this->senderIp, this->senderPort).cid);
				sendConnectionAccept(cid);
			}

			inline void handlePing(CID mCID)
			{
				cManager.pingReceived(mCID);
			}

			inline void handleSyncRequest(CID mCID)
			{
				auto clientSnapshot(this->template popRecv<Diff>());

				auto result(this->syncManager.getDiffWith(clientSnapshot));

				this->debugLo()	<< "Calculating diff: \n" 
 								<< result.toJson() 
 								<< "\n";

 				sendToClient<SPT::SyncRequestSatisfy>(mCID, result);

 				this->debugLo() << "Sent diff \n";


				/*auto revisionID(this->template popRecv<RevisionID>());

				if(this->syncManager.getRevisionID() != revisionID)
				{
					// TODO: send changed
				}
				else
				{
					// No need to send anything
					sendToClient<SPT::SyncRequestUnneeded>();
				}*/
			}

			inline void sendConnectionAccept(CID mCID)
			{
				sendToClient<SPT::ConnectionAccept>(mCID, mCID, "Welcome to the server.");
			}

		public:
			inline SessionServer(syn::Port mPort) : Impl::SessionServerBase<TSettings>{"Server", mPort}
			{
				this->setBusy(true);
			}
	};
}

#endif
