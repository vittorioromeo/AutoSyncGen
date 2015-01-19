#ifndef AUTOSYNCGEN_SNAPSHOT
#define AUTOSYNCGEN_SNAPSHOT

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"

namespace syn
{
	namespace Impl
	{
		template<typename TManager> struct Snapshot
		{
			using ObjBitset = typename TManager::ObjBitset;
			using BitsetStorage = typename TManager::BitsetStorage;	

			BitsetStorage bitsetIDs;			
			ssvu::TplRepeat<DiffTypeData, TManager::typeCount> snapshotTypeDatas;

			inline auto toJson() const
			{
				using namespace ssvj;

				Val result{Arr{}};
				result.emplace(Arr{}); // Bitset array
				result.emplace(Arr{}); // Data array

				for(const auto& b : bitsetIDs) result[0].emplace(b.to_string());
				ssvu::tplFor([this, &result](const auto& mI){ result[1].emplace(mI.toJson()); }, snapshotTypeDatas);
				return result;
			}x
		};
	}
}

#endif