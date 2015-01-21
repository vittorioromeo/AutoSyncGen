#ifndef AUTOSYNCGEN_DIFF
#define AUTOSYNCGEN_DIFF

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"

// TODO:
// 1) client sends SNAPSHOT
// 2) client receives DIFF
// 3) client applies DIFF to MANAGER

// ???
// 1) client gets initial server SNAPSHOT
// 2) client asks server for changes
// 3) server sends diff and applies the same changes to internally stored client snapshot (store it in connection data)
// 4) client asks server for changes... repeat

namespace syn
{
	namespace Impl
	{
		template<typename TManager> struct Diff
		{
			using ObjBitset = typename TManager::ObjBitset;
			using BitsetStorage = typename TManager::BitsetStorage;
			using TypeData = DiffTypeData;

			ssvu::TplRepeat<TypeData, TManager::typeCount> typeDatas;

			inline auto toJson() const
			{
				using namespace ssvj;

				Val result{Arr{}};

				ssvu::tplFor([this, &result](const auto& mI){ result.emplace(mI.toJson()); }, typeDatas);
				return result;
			}

			inline void initFromJson(const ssvj::Val& mX)
			{
				using namespace ssvj;

				ssvu::tplForIdx([this, &mX](auto mIdx, auto& mTD){ mTD.initFromJson(mX[mIdx]); }, typeDatas);
			}
		};
	}
}

#endif
