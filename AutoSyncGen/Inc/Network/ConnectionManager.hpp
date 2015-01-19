#ifndef AUTOSYNCGEN_CONNECTIONMANAGER
#define AUTOSYNCGEN_CONNECTIONMANAGER

namespace syn
{
	namespace Internal
	{
		constexpr int maxSecondsUntilTimeout{5};

		struct ConnectionData
		{
			CID cid;
			syn::IpAddress ip;
			syn::Port port;
			int secondsUntilTimeout;
		};

		class ConnectionManager
		{
			private:
				CID nextCID{0};
				std::map<CID, ConnectionData> connections;
				std::future<void> timeoutThread;
				bool busy{true};

			public:
				inline ConnectionManager()
				{
					timeoutThread = std::async(std::launch::async, [this]
					{
						std::vector<CID> toDisconnect;

						while(busy)
						{
							toDisconnect.clear();	

							for(auto& p : connections)
							{
								auto& c(p.second);

								--(c.secondsUntilTimeout);
								if(c.secondsUntilTimeout == 0)
								{
									toDisconnect.emplace_back(c.cid);
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
					auto cidToUse(nextCID++);
					connections[cidToUse] = ConnectionData{cidToUse, mIp, mPort, maxSecondsUntilTimeout};
					return connections[cidToUse];
				}

				inline void forceDisconnect(CID mCID)
				{
					SSVU_ASSERT(has(mCID));

					connections.erase(mCID);

					ssvu::lo("TODO") << "Client " << mCID << " was disconnected\n";
				}

				inline bool has(CID mCID) const noexcept
				{
					return connections.find(mCID) != std::end(connections); 
				}

				inline auto& operator[](CID mCID) noexcept
				{
					SSVU_ASSERT(has(mCID));
					return connections[mCID];
				}

				inline void pingReceived(CID mCID) noexcept
				{
					SSVU_ASSERT(has(mCID));
					connections[mCID].secondsUntilTimeout = maxSecondsUntilTimeout;
				}	
		};	
	}
}

#endif
