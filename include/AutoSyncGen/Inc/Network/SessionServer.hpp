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
			using Snapshot = typename BaseType::Snapshot;

		private:
			Impl::ClientHandlerManager cManager;

			template<SPT TType, typename... TArgs> inline void sendToClient(CID mCID, TArgs&&... mArgs)
			{
				if(!cManager.has(mCID))
				{
					this->debugLo() << "Wanted to send a packet to CID " << mCID << ", but it's not connected\n";
					return;
				}

				this->template mkPacket<TType>(FWD(mArgs)...);


				const auto& ch(cManager[mCID]);
				this->sendTo(ch->getIp(), ch->getPort());
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
					case RPT::Data:
						this->onDataReceived(cid, this->recvBuffer);
						return;
					case RPT::SyncSatisfied:
						// TODO
						return;
				}
			}

			inline void handleConnectionRequest()
			{
				this->debugLo() << "Connection request received\n";
				this->debugLo() << "Accepting request\n";

				auto cid(cManager.acceptClient(this->senderIp, this->senderPort).getCID());
				sendConnectionAccept(cid);
			}

			inline void handlePing(CID mCID)
			{
				cManager.pingReceived(mCID);
			}

			inline void handleSyncRequest(CID mCID)
			{
				auto clientSnapshot(this->template popRecv<Snapshot>());

				// TODO: only create snapshot if required (cache snapshot)
				auto serverSnapshot(this->syncManager.getSnapshot());

		//		this->debugLo() << "SERVR" << serverSnapshot.toJson() << "\n\n";

				auto result(serverSnapshot.getDiffWith(clientSnapshot));
				auto resultJson(result.toJson());

				if(!result.isEmpty()) this->debugLo() << "Sent diff: \n" << resultJson << "\n";

				sendToClient<SPT::SyncRequestSatisfy>(mCID, result);


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
			ssvu::Delegate<void(CID, sf::Packet&)> onDataReceived;

			inline SessionServer(syn::Port mPort) : Impl::SessionServerBase<TSettings>{"Server", mPort}
			{
				this->setBusy(true);
			}

			template<typename... TArgs> inline void sendDataToClient(CID mCID, TArgs&&... mArgs)
			{
				sendToClient<SPT::Data>(mCID, FWD(mArgs)...);
			}
	};
}

#endif
