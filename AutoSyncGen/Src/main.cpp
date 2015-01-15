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

inline void fillPacket(sf::Packet&)
{

}
template<typename T, typename... TArgs> inline void fillPacket(sf::Packet& mPacket, T&& mArg, TArgs&&... mArgs)
{
	mPacket << mArg;
	fillPacket(mPacket, ssvu::fwd<TArgs>(mArgs)...);
}

template<typename TPacketType> class PacketHandler
{
	private:
		using HandlerFunc = ssvu::Func<void(sf::Packet&)>;
		std::map<TPacketType, HandlerFunc> funcs;
		sf::Packet& buffer;

	public:
		inline PacketHandler(sf::Packet& mBuffer) : buffer{mBuffer} 
		{ 

		}

		inline void handle(sf::Packet& mPacket)
		{
			PacketType type{-1};

			try
			{
				mPacket >> type;

				auto itr(funcs.find(type));
				if(itr == std::end(funcs))
				{
					ssvu::lo("PacketHandler") << "Can't handle packet of type: " << type << std::endl;
					return;
				}

				itr->second(mPacket);
			}
			catch(std::exception& mEx)
			{
				ssvu::lo("PacketHandler") << "Exception during packet handling: (" << type << ")\n" << mEx.what() << std::endl;
			}
			catch(...)
			{
				ssvu::lo("PacketHandler") << "Unknown exception during packet handling: (" << type << ")\n";
			}
		}

		auto& operator[](TPacketType mType) { return funcs[mType]; }
};

class ConsoleSessionController;

template<typename TSPT, typename TRPT> class SessionHost
{
	public:	
		using SPT = TSPT;
		using RPT = TRPT;

	private:
		std::string name;
		syn::IpAddress ip;
		syn::Port port;
		sf::UdpSocket socket;	

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

		ssvu::SizeT packetNext;

	protected:
		sf::Packet buffer;
		std::future<void> hostFuture;
		PacketHandler<RPT> handler;

		template<SPT TType, typename... TArgs> inline void mkPacket(TArgs&&... mArgs)
		{
			buffer.clear();
			buffer << static_cast<PacketType>(TType);
			fillPacket(buffer, ssvu::fwd<TArgs>(mArgs)...);
		}	

	public:
		inline SessionHost(std::string mName, syn::Port mPort) : name{mName}, ip{syn::IpAddress::getLocalAddress()}, port{mPort}, handler{buffer}
		{
			socket.setBlocking(true);
		}

		inline const auto& getName() const noexcept { return name; }
		inline const auto& getIp() const noexcept { return ip; }
		inline const auto& getPort() const noexcept { return port; }

		inline auto debugLo() -> decltype(ssvu::lo(name))
		{
			// TODO: ssvu::getNullLogStream(); for fake logs
			return ssvu::lo(name);
		}
};

class SessionServer : public SessionHost<PacketStoC, PacketCtoS>
{
	private:

	public:
		inline SessionServer(syn::Port mPort) : SessionHost{"Server", mPort} 
		{

		}
};

class SessionClient : public SessionHost<PacketCtoS, PacketStoC>
{
	public:
		static constexpr int nullClientID{-1};

	private:
		syn::IpAddress serverIp;
		syn::Port serverPort;

		// Assigned from server after connection is accepted
		int clientID{nullClientID};

		inline void sendConnectionRequest()
		{
			mkPacket<SPT::ConnectionRequest>();
		}
		
	public:
		inline SessionClient(syn::Port mPort, syn::IpAddress mServerIp, syn::Port mServerPort) 
			: SessionHost{"Client", mPort}, serverIp{mServerIp}, serverPort{mServerPort}
		{
			handler[RPT::ConnectionAccept] = [this](sf::Packet& mP)
			{

			};
		}

};

class Session
{
	friend class ConsoleSessionController;

	private:
		enum class Role{Server, Client};

		Role role;
		syn::IpAddress ip;
		syn::Port port;
		sf::UdpSocket socket;

		std::future<void> socketFuture;

		static constexpr ssvu::SizeT bufferSize{2048};
		std::array<char, 2048> buffer;

		inline void tryBindSocket()
		{
			if(socket.bind(port) != sf::Socket::Done)
			{
				throw std::runtime_error("Error binding socket");
			}
			else
			{
				ssvu::lo() << "Socket successfully bound to port " + ssvu::toStr(port) + "\n";
			}
		}

		inline void serverFutureImpl()
		{
			syn::IpAddress senderIp;
			syn::Port senderPort;
			ssvu::SizeT receivedSize;

			while(true)
			{
				if(socket.receive(buffer.data(), bufferSize, receivedSize, senderIp, senderPort) != sf::Socket::Done)
				{
				    ssvu::lo("Server") 	<< "Socket receive error: \n" 
				  						<< "	Sender: " << senderIp << ":" << senderPort << "\n";

				  	continue;
				}

				ssvu::lo("Server") 	<< "Received " << receivedSize << " bytes from: \n" 
				  					<< "	Sender: " << senderIp << ":" << senderPort << "\n";
			}
		}

		inline void clientFutureImpl()
		{
			while(true)
			{

			}
		}

		inline void startSocketFuture()
		{
			if(role == Role::Server)
			{
				socketFuture = std::async(std::launch::async,  [this]{ serverFutureImpl(); });
			}
			else
			{
				socketFuture = std::async(std::launch::async,  [this]{ clientFutureImpl(); });
			}
		}

	

	public:
		inline Session()
		{
			socket.setBlocking(true);
		}

		inline void join()
		{
			socketFuture.get();
		}	
};

class ConsoleSessionController
{
	private:
		Session& session;

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
					session.role = Session::Role::Server;
					ssvu::lo() << "Server selected\n";
					break;
				}
				else if(choice == 1)
				{
					session.role = Session::Role::Client;
					ssvu::lo() << "Client selected\n";
					break;
				}
				else
				{
					ssvu::lo() << "Invalid selection\n";
					continue;
				}
			}
		}

		inline void selectServer()
		{
			while(true)
			{
				selectPort();

				break;
			}
		}

		inline void selectClient()
		{
			while(true)
			{
				selectIP();
				selectPort();

				break;
			}
		}

		inline void selectIP()
		{
			while(true)
			{
				// TODO: check validity
				// TODO: display "sender" or "receiver"

				ssvu::lo() << "Insert ip address: \n";
				std::cin >> session.ip;

				break;
			}
		}

		inline void selectPort()
		{
			while(true)
			{
				// TODO: check validity
				// TODO: display "sender" or "receiver"

				ssvu::lo() << "Insert port: \n";
				std::cin >> session.port;

				break;
			}
		}

	public:
		inline ConsoleSessionController(Session& mSession) noexcept : session{mSession}
		{

		}

		inline void start()
		{
			selectRole();
			
			if(session.role == Session::Role::Server)
			{
				selectServer();
			}
			else
			{
				selectClient();
			}

			session.tryBindSocket();
			session.startSocketFuture();
		}
};


int main()
{	
	Session s;
	ConsoleSessionController cs{s};

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

	s.join();

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
