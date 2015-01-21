#ifndef AUTOSYNCGEN_PACKETTYPES
#define AUTOSYNCGEN_PACKETTYPES

namespace syn
{
	namespace PT
	{
		using NType = signed char;

		enum class CtoS : NType
		{
			ConnectionRequest = 0,		// Request to establish a connection to the server
			Ping = 1,					// Ping to avoid timing out with the server


			SyncRequest = 2,			// Request to sync data with server
			SyncSatisfied = 3,			// TODO: implement (sucessfully syncronized with server)

			Data = 4,
		};

		enum class StoC : NType
		{
			ConnectionAccept = 0,		// Accept a requested client connection, assigning the client a CID
			ConnectionDecline = 1,		// Decline a client's requested connection


			SyncRequestSatisfy = 2,		// Satisfy sync request (if the client's revision is behind)
			SyncRequestUnneeded = 3,	// Sync not needed (client has the same revision as the server)
			SyncRequestDecline = 4,		// Sync request declined (overload/technical issue)

			Data = 5
		};
	}

	// TODO: optimization opportunities
	inline Packet& operator<<(Packet& mP, const ssvj::Val& mX)
	{
		return mP << mX.getWriteToStr<ssvj::WSMinified>();
	}
	inline Packet& operator>>(Packet& mP, ssvj::Val& mX)
	{
		std::string str;
		mP >> str;
		mX.readFromStr(str);
		return mP;
	}

	template<typename TManager> inline Packet& operator<<(Packet& mP, const Impl::Diff<TManager>& mX)
	{
		return mP << mX.toJson();
	}
	template<typename TManager> inline Packet& operator>>(Packet& mP, Impl::Diff<TManager>& mX)
	{
		ssvj::Val data{};
		mP >> data;
		mX.initFromJson(data);
		return mP;
	}

	template<typename TManager> inline Packet& operator<<(Packet& mP, const Impl::Snapshot<TManager>& mX)
	{
		return mP << mX.toJson();
	}
	template<typename TManager> inline Packet& operator>>(Packet& mP, Impl::Snapshot<TManager>& mX)
	{
		ssvj::Val data{};
		mP >> data;
		mX.initFromJson(data);
		return mP;
	}

	/*
	inline Packet& operator<<(Packet& mP, const PT::NType& mPT)	{ return mP << PT::NType(mPT); }
	inline Packet& operator>>(Packet& mP, PT::NType& mPT)		{ return mP >> reinterpret_cast<PT::NType&>(mPT); }
	inline Packet& operator<<(Packet& mP, const PT::StoC& mPT)	{ return mP << PT::NType(mPT); }
	inline Packet& operator>>(Packet& mP, PT::StoC& mPT)		{ return mP >> reinterpret_cast<PT::NType&>(mPT); }
	inline Packet& operator<<(Packet& mP, const PT::CtoS& mPT)	{ return mP << PT::NType(mPT); }
	inline Packet& operator>>(Packet& mP, PT::CtoS& mPT)		{ return mP >> reinterpret_cast<PT::NType&>(mPT); }
	*/
}

#endif
