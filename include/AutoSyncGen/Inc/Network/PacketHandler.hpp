#ifndef AUTOSYNCGEN_PACKETHANDLER
#define AUTOSYNCGEN_PACKETHANDLER

namespace syn
{
    namespace Impl
    {
        inline void fillPacket(Packet&) noexcept {}
        template <typename T, typename... TArgs>
        inline void fillPacket(Packet& mP, T&& mArg, TArgs&&... mArgs)
        {
            mP << mArg;
            fillPacket(mP, FWD(mArgs)...);
        }

        /*template<typename THost> class PacketHandler
        {
            private:
                using HandlerFunc = void(THost::*)();
                std::map<PacketType, HandlerFunc> funcs;
                THost& host;

                using RPT = typename THost::RPT;

                inline auto& getRecvBuffer() noexcept { return host.recvBuffer;
        }
                inline auto& debugLo() noexcept { return host.debugLo(); }

            public:
                inline PacketHandler(THost& mHost) : host{mHost}
                {

                }

                inline void handle()
                {
                    PacketType type;

                    try
                    {
                        getRecvBuffer() >> type;
                        auto baseType(type);

                        auto itr(funcs.find(baseType));
                        if(itr == std::end(funcs))
                        {
                            debugLo() << "Can't handle packet of type: " << type
        <<
        std::endl;
                            return;
                        }

                        auto fnPtr(itr->second);
                        (host.*fnPtr)();
                    }
                    catch(std::exception& mEx)
                    {
                        debugLo() << "Exception during packet handling: (" <<
        type
        << ")\n" << mEx.what() << std::endl;
                    }
                    catch(...)
                    {
                        debugLo() << "Unknown exception during packet handling:
        ("
        << type << ")\n";
                    }
                }

                auto& operator[](RPT mType) { return funcs[mType]; }
        };*/
    }
}

#endif
