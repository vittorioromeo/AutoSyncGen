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
		template<typename TManager> struct SnapshotTypeData
		{
			std::map<ID, ssvj::Val> items;

			inline auto toJson() const
			{
				using namespace ssvj;

				// TODO: better syntax in ssvj
				Val result{Obj{}};			
				for(const auto& x : items) result[ssvu::toStr(x.first)] = x.second;
			
				return result;
			}

			inline void initFromJson(const ssvj::Val& mX)
			{
					// TODO
				// TODO 
			}
		};

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
			}
		};

		template<typename TManager> struct Diff
		{
			using ObjBitset = typename TManager::ObjBitset;
			using BitsetStorage = typename TManager::BitsetStorage;

			BitsetStorage bitsetIDs;
			ssvu::TplRepeat<DiffTypeData, TManager::typeCount> diffTypeDatas;

			inline auto toJson() const
			{
				using namespace ssvj;

				Val result{Arr{}};
				result.emplace(Arr{}); // Bitset array
				result.emplace(Arr{}); // Data array

				for(const auto& b : bitsetIDs) result[0].emplace(b.to_string());
				ssvu::tplFor([this, &result](const auto& mI){ result[1].emplace(mI.toJson()); }, diffTypeDatas);
				return result;
			}

			inline void initFromJson(const ssvj::Val& mX)
			{
				using namespace ssvj;

				for(auto i(0u); i < bitsetIDs.size(); ++i) bitsetIDs[i] = ObjBitset{mX[0][i].as<std::string>()};				
				ssvu::tplForIdx([this, &mX](auto mIdx, auto& mDTD){ mDTD.initFromJson(mX[1].as<Arr>()[mIdx]); }, diffTypeDatas);
			}
		};
	}
}

#endif