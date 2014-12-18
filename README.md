# AutoSyncGen

*UNIME 2014-2015 Networking course project*

---

**AutoSyncGen** is a C++14 library that helps with automatic compile-time generation of generic synchronizable data structures over a network.

By defining data structures that inherit from `SyncObj`, an underlying synchonizable object is generated. 

The `SyncObj` contains a bitset that keeps track of the modified fields. 

Synchronizable objects are stored in a manager and have an unique ID. 

Both the server and the client create an instance of the manager.

Itis possible to serialize the state of the entire manager in **Json**, so that it can be sent over the network.

It's also possible to automatically generate and serialize the difference between two manager states. The idea is to make sure only the fields that have changed are sent over the network.

---

Example usage:

```
// `Sphere` is a synchronizable object. 
// The fields specified as template parameters of `syn::SyncObj` are 
// synchronizable fields.

struct Sphere : syn::SyncObj
<
	float,			// X
	float,			// Y
	float,			// Z
	float,			// Radius
	int,			// Color (R)
	int,			// Color (G)
	int,			// Color (B)
	std::string		// Label
>
{
	// Lightweight proxy objects can be added to `Sphere` to allow the programmer
	// to perform basic actions on synchronizable fields with a familiar syntax.

	ProxyAt<0> x{get<0>()};
	ProxyAt<1> y{get<1>()};
	ProxyAt<2> z{get<2>()};
	ProxyAt<3> radius{get<3>()};
	ProxyAt<4> colorR{get<4>()};
	ProxyAt<5> colorG{get<5>()};
	ProxyAt<6> colorB{get<6>()};
	ProxyAt<7> label{get<7>()};
}; 

// It is necessary to create a lifetime manager that tells the synchronizable manager 
// how to create and delete object instances.
template<typename> struct LifetimeManager;

// By specializing the lifetime manager, we can specify how to store objects of 
// a certain type.
template<> struct LifetimeManager<Sphere>
{
	using Handle = Sphere*;

	Handle getNullHandle() noexcept { return nullptr; }
	std::vector<std::unique_ptr<Sphere>> storage;

	inline Handle create()
	{
		storage.emplace_back(std::make_unique<Sphere>());
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

int main()
{
	// Instantiate two synchronization managers that simulate server-client communication.
	// The first template parameter is the lifetime manager type, the rest of parameters 
	// are the types of all synchronizable objects.
	syn::SyncManager<LifetimeManager, Sphere> server;
	syn::SyncManager<LifetimeManager, Sphere> client;

	// Create a sphere on the server with some data.
	auto h1(server.serverCreate<Sphere>(/* ... data here ... */));

	// Get the diff between the state of the client and the server.
	auto diff(server.getDiffWith(client)); 

	// Convert the diff to Json, to send it over the network.
	auto diffJson(server.getDiffWith(client).toJson()); 

	/* ... send the diff here ... */

	// Apply the received diff to the client.
	// Objects are created/removed/updated on the client.
	client.applyDiff(diff);

	return 0;
}
```