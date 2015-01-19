#include "../../AutoSyncGen/Inc/AutoSyncGen.hpp"

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

using SyncManagerType = syn::SyncManager<LifetimeManager, TestPlayer, TestEnemy>;
using Settings = syn::SessionSettings<SyncManagerType>;

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
							<< "    2. Exit\n"
							<< std::endl;

				std::cin >> choice;

				if(choice == 0)
				{					
					ssvu::lo() << "Server selected\n";
					selectServer();	
					return;
				}
				if(choice == 1)
				{					
					ssvu::lo() << "Client selected\n";
					selectClient();
					return;
				}
				if(choice == 2)
				{					
					return;
				}
				
				ssvu::lo() << "Invalid selection\n";
			}
		}

		inline void selectServer()
		{
			auto port = getInputPort("Insert port to listen onto: \n", 27015);
			
			syn::SessionServer<Settings> server{port};

			while(server.isBusy())
			{

			}
		}

		inline void selectClient()
		{
			auto port = getInputPort("Insert port to listen onto: \n", 27016);

			ssvu::lo() << "Enter target server ip and port: \n";
			auto serverIp = getInputIp("Insert target server ip: \n", syn::IpAddress{"127.0.0.1"});
			auto serverPort = getInputPort("Insert target server port: \n", 27015);
			
			syn::SessionClient<Settings> client{port, serverIp, serverPort};

			while(client.isBusy())
			{

			}
		}

		template<typename T> inline int getChoice(const std::string& mMsg, const T& mDefault)
		{
			ssvu::lo() 	<< mMsg
						<< "Choose: \n" 
						<< "	0. Default (" << mDefault << ")\n"
						<< "	1. Enter manually\n";

			int choice{0};
			std::cin >> choice;

			return choice;
		}

		inline syn::IpAddress getInputIp(const std::string& mMsg, const syn::IpAddress& mDefault)
		{
			auto choice(getChoice(mMsg, mDefault));
			if(choice == 0) return mDefault;

			syn::IpAddress result;
			std::cin >> result;
			return result;
		}

		inline syn::Port getInputPort(const std::string& mMsg, const syn::Port& mDefault)
		{
			auto choice(getChoice(mMsg, mDefault));
			if(choice == 0) return mDefault;

			syn::Port result;
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
