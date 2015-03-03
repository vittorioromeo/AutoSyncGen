#include <SSVUtils/Test/Test.hpp>
#include <SSVUtils/Tests/Tests.hpp>
#include "../../AutoSyncGen/Inc/AutoSyncGen.hpp"

#define SYN_PROXY(mIdx, mName) ProxyAt<mIdx> mName{get<mIdx>()}
/*
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
		storage.emplace_back(ssvu::mkUPtr<TestPlayer>());
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
		storage.emplace_back(ssvu::mkUPtr<TestEnemy>());
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
*/

struct Message : syn::SyncObj
<
	int,			// messageID
	std::string,	// author
	std::string		// contents
>
{
	SYN_PROXY(0, messageID);
	SYN_PROXY(1, author);
	SYN_PROXY(2, contents);
/*
	inline Message(int mMessageID, const std::string& mAuthor, const std::string& mContents)
	{
		messageID = mMessageID;
		author = mAuthor;
		contents = mContents;
	}
*/
};

template<typename> struct LifetimeManager;

template<> struct LifetimeManager<Message>
{
	using Handle = Message*;

	inline Handle getNullHandle() noexcept { return nullptr; }

	std::vector<ssvu::UPtr<Message>> storage;

	inline Handle create()
	{
		storage.emplace_back(ssvu::mkUPtr<Message>());
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

enum class DP_StoC : int
{
	DisplayMsg = 0
};

enum class DP_CtoS : int
{
	SendMsg = 0,
	EditMsg = 1
};

using namespace syn;

using SyncManagerType = syn::SyncManager<LifetimeManager, Message>;
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

			/*
			auto temp(ssvj::mkObj());
			temp["0"] = 10.f;
			temp["1"] = 25.f;
			temp["2"] = 100;
			temp["3"] = "banana";
*/

//			auto h1(server.getSyncManager().serverCreate<TestPlayer>(temp));

			//std::map<int, Message*> messages;
			int lastID{0};

			server.onDataReceived += [&server, &lastID](syn::CID /* TODO mCID */, syn::Packet& mP)
			{
				DP_CtoS type;
				mP >> type;

				if(type == DP_CtoS::SendMsg)
				{
					std::string author;
					std::string msg;

					mP >> author >> msg;

					int id{lastID++};

					auto temp(ssvj::mkObj());
					temp["0"] = id;
					temp["1"] = author;
					temp["2"] = msg;

					server.getSyncManager().serverCreate<Message>(temp);

					//auto handle(server.getSyncManager().serverCreate2<Message>(id, author, msg));
					//messages[id] = handle;
				}
				else if(type == DP_CtoS::EditMsg)
				{

				}
			};

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


			std::string author;
			ssvu::lo() << "Enter your name: \n";
			std::cin >> author;

			syn::SessionClient<Settings> client{port, serverIp, serverPort};
			client.onDataReceived += [](syn::Packet& mP)
			{
				DP_StoC type;
				mP >> type;

				if(type == DP_StoC::DisplayMsg)
				{
					int id;
					std::string author_msg;
					std::string msg;

					mP >> id >> author_msg >> msg;

					ssvu::lo("MSG")	<< "ID: " << id << "\n"
									<< "Author: " << author_msg << "\n"
									<< msg << "\n\n";
				}
			};

			while(client.isBusy())
			{
				int choice;
				ssvu::lo()	<< "Choose: \n"
							<< "0. New message\n"
							<< "1. Edit message\n"
							<< "2. Show local msg\n"
							<< "3. Disconnect\n";

				std::cin >> choice;

				if(choice == 0)
				{
					std::string msg;

					ssvu::lo() << "Enter message: \n";
					std::cin >> msg;

					client.sendDataToServer(DP_CtoS::SendMsg, author, msg);

					continue;
				}

				if(choice == 1)
				{
					int id;
					std::string msg;

					ssvu::lo() << "Enter message ID: \n";
					std::cin >> id;

					ssvu::lo() << "Enter message: \n";
					std::cin >> msg;

					client.sendDataToServer(DP_CtoS::EditMsg, author, msg, id);

					continue;
				}

				if(choice == 2)
				{
					ssvu::lo() << client.getSyncManager().getSnapshot().toJson() << "\n\n";


					continue;
				}

				if(choice == 3)
				{
					// TODO:


					break;
				}
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
	SSVUT_RUN();

	ConsoleSessionController cs;
	cs.start();



	return 0;

/*
	syn::SyncManager<LifetimeManager, TestPlayer, TestEnemy> server;
	syn::SyncManager<LifetimeManager, TestPlayer, TestEnemy> client;

	auto temp(ssvj::mkObj());
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
*/
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


// TODO: only send bitsets for sync
