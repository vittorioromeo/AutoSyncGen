#ifndef AUTOSYNCGEN_SNAPSHOT
#define AUTOSYNCGEN_SNAPSHOT

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"
#include "../../AutoSyncGen/Inc/SnapshotTypeData.hpp"
#include "../../AutoSyncGen/Inc/DiffTypeData.hpp"
#include "../../AutoSyncGen/Inc/Diff.hpp"

namespace syn
{
	namespace Impl
	{
		template<typename TManager> struct Snapshot
		{
			using ObjBitset = typename TManager::ObjBitset;
			using BitsetStorage = typename TManager::BitsetStorage;
			using TypeData = SnapshotTypeData;
			using DiffType = Impl::Diff<TManager>;

			BitsetStorage bitsetIDs;
			ssvu::TplRepeat<TypeData, TManager::typeCount> typeDatas;

			inline static auto getJsonDiff(const ssvj::Val& mX, const ssvj::Val& mY)
			{
				ssvj::Val result{ssvj::Obj{}};
				ObjBitset xFieldFlags{mX[jsonFieldFlagsKey].as<ssvj::Str>()};

				for(auto i(0u); i < xFieldFlags.size(); ++i)
				{
					if(!xFieldFlags[i])	continue;

					std::string iStr{ssvu::toStr(i)};
					result[iStr] = mY[iStr];
				}

				return result;
			}

			inline auto toJson() const
			{
				using namespace ssvj;

				// TODO: better ssvj syntax?
				Val result{Arr{}};
				result.emplace(Arr{}); // Bitset array
				result.emplace(Arr{}); // Data array

				// TODO: bitset serialization in ssvj
				for(const auto& b : bitsetIDs) result[0].emplace(b.to_string());
				ssvu::tplFor([this, &result](const auto& mI){ result[1].emplace(mI.toJson()); }, typeDatas);
				return result;
			}

			inline void initFromJson(const ssvj::Val& mX)
			{
				using namespace ssvj;

				for(auto i(0u); i < bitsetIDs.size(); ++i) bitsetIDs[i] = ObjBitset{mX[0][i].as<std::string>()};
				ssvu::tplForIdx([this, &mX](auto mIdx, auto& mTD){ mTD.initFromJson(mX[1].as<Arr>()[mIdx]); }, typeDatas);
			}

			inline auto getDiffWith(const Snapshot& mX)
			{
				DiffType result;

				ssvu::tplForIdx([this, &result, &mX](auto mIType, auto& mTDCurrent, auto& mTDOther, auto& mTDDiff) mutable
				{
					// TODO: bitsetIDs need to be cleared somewhere
					// TODO: use ack/nack system?
					const auto& myBitset(bitsetIDs[mIType]);
					const auto& diffBitset(mX.bitsetIDs[mIType]);

					auto diffBitsetToCreate((~diffBitset) & myBitset);
					auto diffBitsetToRemove((~myBitset) & diffBitset);
					auto diffBitsetToUpdate(myBitset & diffBitset);

					for(auto i(0u); i < maxObjs; ++i)
					{
						if(diffBitsetToCreate[i]) mTDDiff.toCreate[i] = mTDCurrent.items[i];
						if(diffBitsetToRemove[i]) mTDDiff.toRemove.emplace_back(i);
						if(diffBitsetToUpdate[i]) mTDDiff.toUpdate[i] = Snapshot<TManager>::getJsonDiff(mTDCurrent.items.at(i), mTDOther.items.at(i));
					}
				}, typeDatas, mX.typeDatas, result.typeDatas);

				return result;
			}
		};
	}
}

#endif
