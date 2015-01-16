#ifndef AUTOSYNCGEN_PACKETTYPES
#define AUTOSYNCGEN_PACKETTYPES

namespace syn
{
	using PacketType = signed char;

	enum PacketCtoS : PacketType
	{
		ConnectionRequest = 0,
		Ping = 1
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
}

#endif
