#include <SSVUtils/Test/Test.hpp>
#include <SSVUtils/Tests/Tests.hpp>
#include "../../AutoSyncGen/Inc/AutoSyncGen.hpp"

/// @brief Synchronizable data structure representing a chat message.
struct Message : syn::SyncObj
<
	// The fields types are passed as a variadic type list.

	int,			// messageID
	std::string,	// author
	std::string		// contents
>
{
	// Field proxies are generated via macros for convenience.

	SYN_PROXY(0, messageID);
	SYN_PROXY(1, author);
	SYN_PROXY(2, contents);
};

// Forward declaration of the lifetime manager template class.
template<typename> struct LifetimeManager;

/// @brief `LifetimeManager` specialization for `Message`.
template<> struct LifetimeManager<Message>
{
	/// @brief Type of the handle used by the `SyncManager`.
	using Handle = Message*;

	/// @brief Required function that returns a null handle.
	inline Handle getNullHandle() noexcept { return nullptr; }

	/// @brief Internal memory storage for `Message` instances.
	std::vector<ssvu::UPtr<Message>> storage;

	/// @brief Required function that creates an object and returns an handle to it.
	inline Handle create()
	{
		return &ssvu::getEmplaceUPtr<Message>(storage);
	}

	/// @brief Required function that removes an object attached to a specific handle.
	inline void remove(Handle mHandle)
	{
		ssvu::eraseRemoveIf(storage, [this, mHandle](const auto& mUPtr)
		{
			return mUPtr.get() == mHandle;
		});
	}
};

/// @brief Custom unique types for server-to-client chat packets.
enum class DP_StoC : int
{
	DisplayMsg = 0
};

/// @brief Custom unique types for client-to-server chat packets.
enum class DP_CtoS : int
{
	SendMsg = 0,
	EditMsg = 1
};

using namespace syn;

// Type of `SyncManager` used for the chat application.
using SyncManagerType = syn::SyncManager<LifetimeManager, Message>;

// Default settings for the `SyncManager`.
using Settings = syn::SessionSettings<SyncManagerType>;

/// @brief Helper function that gets a value from console input.
template<typename T> inline auto safeCin()
{
	T temp;
	std::cin >> temp;
	std::cin.clear();
	std::cin.ignore(10000, '\n');

	return temp;
}

/// @brief Console responsive GUI to control the chat.
class ConsoleSessionController
{
	private:
		inline void selectRole()
		{
			while(true)
			{
				ssvu::lo()	<< "Select: \n"
							<< "    0. Server\n"
							<< "    1. Client\n"
							<< "    2. Exit\n"
							<< std::endl;

				auto choice(safeCin<int>());

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

			int lastID{0};

			server.onDataReceived += [&server, &lastID](syn::CID mCID, syn::Packet& mP)
			{
				DP_CtoS type;
				mP >> type;

				if(type == DP_CtoS::SendMsg)
				{
					std::string author, msg;
					mP >> author >> msg;

					int id{lastID++};

					auto temp(ssvj::mkObj
					(
						"0", id,
						"1", author,
						"2", msg
					));

					auto handle(server.getSyncManager().serverCreate<Message>(temp));
					server.debugLo() << "Message from (" << mCID << "): << " << handle->author.view() << ": " << handle->contents.view() << " >>\n";
				}
				else if(type == DP_CtoS::EditMsg)
				{
					std::string author, msg;
					int id;

					mP >> author >> id >> msg;

					auto& sm(server.getSyncManager());

					LifetimeManager<Message>::Handle h(sm.getHandleFor<Message>(id));
					h->contents.edit() = msg;
				}
			};

			while(server.isBusy())
			{
				ssvu::lo()	<< "Choose: \n"
							<< "    0. Shutdown server\n";

				auto choice(safeCin<int>());

				if(choice == 0)
				{
					return;
				}
			}
		}

		inline void selectClient()
		{
			auto port = getInputPort("Insert port to listen onto: \n", 27016);

			ssvu::lo() << "Enter target server ip and port: \n";
			auto serverIp = getInputIp("Insert target server ip: \n", syn::IpAddress{"127.0.0.1"});
			auto serverPort = getInputPort("Insert target server port: \n", 27015);

			ssvu::lo() << "Enter your name: \n";
			auto clientName(safeCin<std::string>());

			syn::SessionClient<Settings> client{port, serverIp, serverPort};
			client.onDataReceived += [](syn::Packet& mP)
			{
				DP_StoC type;
				mP >> type;

				if(type == DP_StoC::DisplayMsg)
				{
					int id;
					std::string author;
					std::string msg;

					mP >> id >> author >> msg;

					ssvu::lo("MSG")	<< "ID: " << id << "\n"
									<< "Author: " << author << "\n"
									<< msg << "\n\n";
				}
			};

			while(client.isBusy())
			{
				ssvu::lo()	<< "Choose: \n"
							<< "    0. New message\n"
							<< "    1. Edit message\n"
							<< "    2. Show local data\n"
							<< "    3. Disconnect\n";

				auto choice(safeCin<int>());

				if(choice == 0)
				{
					ssvu::lo() << "Enter message: \n";
					auto msg(safeCin<std::string>());

					client.sendDataToServer(DP_CtoS::SendMsg, clientName, msg);

					continue;
				}

				if(choice == 1)
				{
					ssvu::lo() << "Enter message ID: \n";
					auto id(safeCin<int>());

					ssvu::lo() << "Enter message: \n";
					auto msg(safeCin<std::string>());

					client.sendDataToServer(DP_CtoS::EditMsg, clientName, id, msg);

					continue;
				}

				if(choice == 2)
				{
					ssvu::lo() << client.getSyncManager().getSnapshot().toJson() << "\n\n";
					continue;
				}

				if(choice == 3) break;
			}
		}

		template<typename T> inline int getChoice(const std::string& mMsg, const T& mDefault)
		{
			ssvu::lo()	<< mMsg
						<< "Choose: \n"
						<< "	0. Default (" << mDefault << ")\n"
						<< "	1. Enter manually\n";

			return safeCin<int>();
		}

		inline syn::IpAddress getInputIp(const std::string& mMsg, const syn::IpAddress& mDefault)
		{
			auto choice(getChoice(mMsg, mDefault));
			if(choice == 0) return mDefault;

			return safeCin<syn::IpAddress>();
		}

		inline syn::Port getInputPort(const std::string& mMsg, const syn::Port& mDefault)
		{
			auto choice(getChoice(mMsg, mDefault));
			if(choice == 0) return mDefault;

			return safeCin<syn::Port>();
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

	ConsoleSessionController{}.start();
	return 0;
}
