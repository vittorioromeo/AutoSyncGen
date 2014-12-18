#ifndef AUTOSYNCGEN_COMMON
#define AUTOSYNCGEN_COMMON

#include <SSVUtils/Core/Core.hpp>
#include <SSVUtils/Json/Json.hpp>

namespace syn
{
	using Idx = ssvu::SizeT;
	using TypeIdx = ssvu::SizeT;
	using ID = int;
	constexpr ssvu::SizeT maxObjs{100};

	constexpr ssvu::SizeT jsonCreateIdx{0};
	constexpr ssvu::SizeT jsonRemoveIdx{1};
	constexpr ssvu::SizeT jsonUpdateIdx{2};
}

#endif