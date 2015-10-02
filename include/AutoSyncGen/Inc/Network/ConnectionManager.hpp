#ifndef AUTOSYNCGEN_CONNECTIONMANAGER
#define AUTOSYNCGEN_CONNECTIONMANAGER

namespace syn
{
namespace Impl
{
    constexpr int maxSecondsUntilTimeout{5};

    class ClientHandler
    {
        friend class ClientHandlerManager;

    private:
        CID cid{nullCID};
        IpAddress ip;
        Port port;
        int secondsUntilTimeout;
        std::atomic<bool> busy{false};

    public:
        inline void bindToClient(
        CID mCID, const IpAddress& mIp, const Port& mPort)
        {
            cid = mCID;
            ip = mIp;
            port = mPort;
            secondsUntilTimeout = maxSecondsUntilTimeout;

            busy = true;
        }

        inline void unbindFromClient()
        {
            cid = nullCID;
            busy = false;
        }

        inline const auto& getCID() const noexcept { return cid; }
        inline const auto& getIp() const noexcept { return ip; }
        inline const auto& getPort() const noexcept { return port; }
    };

    class ClientHandlerManager
    {
    private:
        CID nextCID{0};
        ssvu::MonoManager<ClientHandler> clientHandlers;
        std::vector<ClientHandler *> chAvailable, chBusy;
        std::map<CID, ClientHandler*> chMap;
        std::future<void> timeoutFuture;
        std::vector<CID> toDisconnect;
        std::atomic<bool> busy{true};

        // std::mutex mtxHandleCollection;

        inline void createClientHandler()
        {
            // std::lock_guard<std::mutex> lg{mtxHandleCollection};

            auto& ch(clientHandlers.create());
            clientHandlers.refresh();
            chAvailable.emplace_back(&ch);
        }

        inline void runTimeout()
        {
            while(busy) {
                for(auto c : chBusy) {
                    --(c->secondsUntilTimeout);

                    if(c->secondsUntilTimeout == 0) c->unbindFromClient();
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

    public:
        inline ClientHandlerManager()
        {
            timeoutFuture = std::async(std::launch::async, [this]
            {
                runTimeout();
            });
        }

        inline ~ClientHandlerManager() { busy = false; }

        inline auto& acceptClient(const IpAddress& mIp, const Port& mPort)
        {
            auto cidToUse(nextCID++);

            toDisconnect.clear();

            for(auto c : chBusy)
                if(!c->busy) {
                    chAvailable.emplace_back(c);
                    toDisconnect.emplace_back(c->cid);
                }

            ssvu::eraseRemoveIf(chBusy, [](const auto& mC)
            {
                return !mC->busy;
            });
            for(auto iId : toDisconnect) chMap.erase(iId);

            if(chAvailable.empty()) createClientHandler();

            auto ch(chAvailable.back());
            chAvailable.pop_back();

            ch->bindToClient(cidToUse, mIp, mPort);
            chMap[cidToUse] = ch;

            return *ch;
        }

        inline bool has(CID mCID) const noexcept
        {
            return chMap.find(mCID) != std::end(chMap);
        }

        inline auto& operator[](CID mCID) noexcept
        {
            SSVU_ASSERT(has(mCID));
            return chMap[mCID];
        }

        inline void pingReceived(CID mCID) noexcept
        {
            SSVU_ASSERT(has(mCID));
            chMap[mCID]->secondsUntilTimeout = maxSecondsUntilTimeout;
        }
    };
}
}

#endif
