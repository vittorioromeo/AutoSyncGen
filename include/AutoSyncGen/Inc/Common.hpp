#ifndef AUTOSYNCGEN_COMMON
#define AUTOSYNCGEN_COMMON

#include <SSVUtils/Core/Core.hpp>
#include <SSVUtils/Json/Json.hpp>
#include <SSVUtils/Delegate/Delegate.hpp>
#include <SSVUtils/MemoryManager/MemoryManager.hpp>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

namespace syn
{
	using ssvu::SizeT;
	using sf::IpAddress;
	using sf::Packet;
	using sf::UdpSocket;

	using Idx = SizeT;
	using TypeIdx = SizeT;
	using ID = int;
	using CID = int;
	using Port = unsigned short;
	using RevisionID = int;

	constexpr CID nullCID{-1};
	constexpr SizeT maxObjs{100};
	constexpr SizeT jsonCreateIdx{0};
	constexpr SizeT jsonRemoveIdx{1};
	constexpr SizeT jsonUpdateIdx{2};

	constexpr const char* jsonFieldFlagsKey{"ff"};
}

#endif
