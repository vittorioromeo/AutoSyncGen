#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/ObjBase.hpp"
#include "../../AutoSyncGen/Inc/FieldProxy.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"	
#include "../../AutoSyncGen/Inc/SyncObj.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"
#include "../../AutoSyncGen/Inc/SyncManager.hpp"

struct TestPlayer : syn::SyncObj
<
	float,			// X
	float,			// Y
	int,			// Health
	std::string		// Name
>
{
	public:
		ProxyAt<0> x;
		ProxyAt<1> y;
		ProxyAt<2> health;
		ProxyAt<3> name;

		inline TestPlayer()
			:   x{get<0>()},
				y{get<1>()},
				health{get<2>()},
				name{get<3>()}
		{

		}
};

struct TestEnemy : syn::SyncObj
<
	float,		// X
	float,		// Y
	int			// Health
>
{
	public:
		ProxyAt<0> x;
		ProxyAt<1> y;
		ProxyAt<2> health;

		inline TestEnemy()
			:   x{get<0>()},
				y{get<1>()},
				health{get<2>()}
		{

		}
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

	inline void update(Handle mHandle)
	{

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

	inline void update(Handle mHandle)
	{

	}
};


int main()
{
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
