#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/Networking.hpp"
#include "../../AutoSyncGen/Inc/ObjBase.hpp"
#include "../../AutoSyncGen/Inc/FieldProxy.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"	
#include "../../AutoSyncGen/Inc/SyncObj.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"
#include "../../AutoSyncGen/Inc/SyncManager.hpp"

#define SYN_PROXY(mIdx, mName) ProxyAt<mIdx> mName{get<mIdx>()}

struct TestPlayer : syn::SyncObj
<
	float,			// X
	float,			// Y
	int,			// Health
	std::string		// Name
>
{
	public:
		SYN_PROXY(0, x);
		SYN_PROXY(1, y);
		SYN_PROXY(2, health);
		SYN_PROXY(3, name);
};

struct TestEnemy : syn::SyncObj
<
	float,		// X
	float,		// Y
	int			// Health
>
{
	public:
		SYN_PROXY(0, x);
		SYN_PROXY(1, y);
		SYN_PROXY(2, health);
};

template<typename T> struct LifetimeManager;

template<> struct LifetimeManager<TestPlayer>
{
	using Handle = TestPlayer*;

	inline Handle getNullHandle() noexcept { return nullptr; }

	std::vector<ssvu::UPtr<TestPlayer>> storage;

	inline Handle create()
	{
		storage.emplace_back(ssvu::makeUPtr<TestPlayer>());
		return storage.back().get();
	}

	inline void remove(Handle mHandle)
	{
		ssvu::eraseRemoveIf(storage, [this, mHandle](const auto& mUPtr)
		{
			return mUPtr.get() == mHandle;
		});
	}
};

template<> struct LifetimeManager<TestEnemy>
{
	using Handle = TestEnemy*;

	inline Handle getNullHandle() noexcept { return nullptr; }

	std::vector<ssvu::UPtr<TestEnemy>> storage;

	inline Handle create()
	{
		storage.emplace_back(ssvu::makeUPtr<TestEnemy>());
		return storage.back().get();
	}

	inline void remove(Handle mHandle)
	{
		ssvu::eraseRemoveIf(storage, [this, mHandle](const auto& mUPtr)
		{
			return mUPtr.get() == mHandle;
		});
	}
};

using PacketType = signed char;

enum PacketCtoS : PacketType
{
	ConnectionRequest = 0,
};

enum PacketStoC : PacketType
{
	ConnectionAccept = 0,
	ConnectionDecline = 1,
};

/*
inline sf::Packet& operator<<(sf::Packet& mP, const PacketType& mPT)	{ return mP << PacketType(mPT); }
inline sf::Packet& operator>>(sf::Packet& mP, PacketType& mPT)			{ return mP >> reinterpret_cast<PacketType&>(mPT); }
inline sf::Packet& operator<<(sf::Packet& mP, const PacketStoC& mPT)	{ return mP << PacketType(mPT); }
inline sf::Packet& operator>>(sf::Packet& mP, PacketStoC& mPT)			{ return mP >> reinterpret_cast<PacketType&>(mPT); }
inline sf::Packet& operator<<(sf::Packet& mP, const PacketCtoS& mPT)	{ return mP << PacketType(mPT); }
inline sf::Packet& operator>>(sf::Packet& mP, PacketCtoS& mPT)			{ return mP >> reinterpret_cast<PacketType&>(mPT); }
*/

inline void fillPacket(sf::Packet&)
{

}
template<typename T, typename... TArgs> inline void fillPacket(sf::Packet& mP, T&& mArg, TArgs&&... mArgs)
{
	mP << mArg;
	fillPacket(mP, ssvu::fwd<TArgs>(mArgs)...);
}

template<typename THost> class PacketHandler
{
	private:
		using HandlerFunc = ssvu::Func<void()>;
		std::map<PacketType, HandlerFunc> funcs;
		THost& host;

		using RPT = typename THost::RPT;

		inline auto& getRecvBuffer() noexcept { return host.recvBuffer; }
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
					debugLo() << "Can't handle packet of type: " << type << std::endl;
					return;
				}

				itr->second();
			}
			catch(std::exception& mEx)
			{
				debugLo() << "Exception during packet handling: (" << type << ")\n" << mEx.what() << std::endl;
			}
			catch(...)
			{
				debugLo() << "Unknown exception during packet handling: (" << type << ")\n";
			}
		}

		auto& operator[](RPT mType) { return funcs[mType]; }
};

class ConsoleSessionController;

template<typename TSPT, typename TRPT> class SessionHost
{
	template<typename> friend class PacketHandler;

	public:	
		using SPT = TSPT;
		using RPT = TRPT;

	private:
		std::string name;
		syn::IpAddress ip;
		syn::Port port;
		sf::UdpSocket socket;	
		bool busy{false};

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

		inline void receiveThread()
		{
			syn::IpAddress senderIp;
			syn::Port senderPort;

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

					handler.handle();	
				}
			}
		}

	protected:
		sf::Packet sendBuffer, recvBuffer;
		std::future<void> hostFuture;
		PacketHandler<SessionHost<TSPT, TRPT>> handler;

		inline void sendTo(const syn::IpAddress& mIp, const syn::Port& mPort)
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
			sendBuffer << static_cast<PacketType>(TType);
			fillPacket(sendBuffer, ssvu::fwd<TArgs>(mArgs)...);
		}	

		inline void setBusy(bool mBusy) noexcept { busy = mBusy; }

	public:
		inline SessionHost(std::string mName, syn::Port mPort) : name{mName}, ip{syn::IpAddress::getLocalAddress()}, port{mPort}, handler{*this}
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
			// TODO: ssvu::getNullLogStream(); for fake logs
			return ssvu::lo(name);
		}
};

using SessionServerBase = SessionHost<PacketStoC, PacketCtoS>;
using SessionClientBase = SessionHost<PacketCtoS, PacketStoC>;

class SessionServer : public SessionServerBase
{
	private:

	public:
		inline SessionServer(syn::Port mPort) : SessionServerBase{"Server", mPort} 
		{
			handler[RPT::ConnectionRequest] = [this]
			{
				debugLo() << "Connection request received\n";
				debugLo() << "Accepting request\n";

			};

			setBusy(true);
		}
};



class SessionClient : public SessionClientBase
{
	public:
		static constexpr int nullClientID{-1};

	private:
		syn::IpAddress serverIp;
		syn::Port serverPort;

		// Assigned from server after connection is accepted
		int clientID{nullClientID};

		template<SPT TType, typename... TArgs> inline void sendToServer(TArgs&&... mArgs)
		{
			mkPacket<TType>(ssvu::fwd<TArgs>(mArgs)...);
			sendTo(serverIp, serverPort);
		}	

		inline void sendConnectionRequest()
		{
			sendToServer<SPT::ConnectionRequest>();
		}
		
	public:
		inline SessionClient(syn::Port mPort, syn::IpAddress mServerIp, syn::Port mServerPort) 
			: SessionClientBase{"Client", mPort}, serverIp{mServerIp}, serverPort{mServerPort}
		{
			handler[RPT::ConnectionAccept] = [this]
			{

			};

			setBusy(true);

			auto xd = std::thread([this]
			{	
				while(true)
				{
					sendConnectionRequest();
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			});
			xd.detach();
		}

};

class ConsoleSessionController
{
	private:
		inline void selectRole()
		{
			while(true)
			{
				int choice{-1};

				ssvu::lo() 	<< "Select: \n"
							<< "    0. Server\n" 
							<< "    1. Client\n"  
							<< std::endl;

				std::cin >> choice;

				if(choice == 0)
				{					
					ssvu::lo() << "Server selected\n";
					selectServer();	
					return;
				}
				else if(choice == 1)
				{					
					ssvu::lo() << "Client selected\n";
					selectClient();
					return;
				}
				
				ssvu::lo() << "Invalid selection\n";
			}
		}

		inline void selectServer()
		{
			auto port = getInputPort();
			
			SessionServer server{port};

			while(server.isBusy())
			{

			}
		}

		inline void selectClient()
		{
			auto port = getInputPort();

			ssvu::lo() << "Enter target server ip and port: \n";
			auto serverIp = getInputIp();
			auto serverPort = getInputPort();
			
			SessionClient client{port, serverIp, serverPort};

			while(client.isBusy())
			{

			}
		}

		inline syn::IpAddress getInputIp()
		{
			syn::IpAddress result;

			ssvu::lo() << "Insert ip: \n";
			std::cin >> result;

			return result;
		}

		inline syn::Port getInputPort()
		{
			syn::Port result;

			ssvu::lo() << "Insert port: \n";
			std::cin >> result;

			return result;
		}

	public:	
		inline void start()
		{
			selectRole();
		}
};


int main()
{	
	ConsoleSessionController cs;
	cs.start();

	


	syn::SyncManager<LifetimeManager, TestPlayer, TestEnemy> server;
	syn::SyncManager<LifetimeManager, TestPlayer, TestEnemy> client;

	ssvj::Val temp{ssvj::Obj{}};
	temp["0"] = 10.f;
	temp["1"] = 25.f;
	temp["2"] = 100;
	temp["3"] = "banana";

	auto h1(server.serverCreate<TestPlayer>(temp));

	ssvu::lo() << server.getDiffWith(client).toJson() << std::endl;
	//ssvu::lo() << client.getDiffWith(server) << std::endl;

	//client.serverCreate<TestPlayer>();
	//ssvu::lo() << server.getDiffWith(client).toJson() << std::endl;

	//h1->x = 100;
	//ssvu::lo() << server.getDiffWith(client).toJson() << std::endl;

	client.applyDiff(server.getDiffWith(client));
	ssvu::lo() << server.getDiffWith(client).toJson() << std::endl;

	auto h1c(client.getHandleFor<TestPlayer>(0));

	ssvu::lo() << h1c->toJsonAll() << std::endl;

	//ssvu::lo() << sizeof h1->x << std::endl;

	return 0;
}


	/*
	TestPlayer player;
	player.x = 10.f;
	player.y = 15.f + player.x;
	player.health = 100;
	player.name = "hello";

	ssvu::lo("JSON_ALL") << "\n" << player.toJsonAll() << "\n";
	ssvu::lo("JSON_CHANGED") << "\n" << player.toJsonChanged() << "\n";

	player.resetFlags();
	ssvu::lo("JSON_CHANGED") << "\n" << player.toJsonChanged() << "\n";

	player.resetFlags();
	player.y = 33.f;
	player.health -= 11;
	if(player.health > 40) player.health /= 2;
	ssvu::lo("JSON_CHANGED") << "\n" << player.toJsonChanged() << "\n";

	player.resetFlags();
	player.x = 11.f;
	player.name = "goodbye";
	ssvu::lo("JSON_CHANGED") << "\n" << player.toJsonChanged() << "\n";
	*/
