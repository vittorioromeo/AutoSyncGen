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
    /// @brief Class representing the snapshot of the entire state of the
    /// `SyncManager`.
    template <typename TManager>
    struct Snapshot
    {
        using ObjBitset = typename TManager::ObjBitset;
        using BitsetStorage = typename TManager::BitsetStorage;
        using TypeData = SnapshotTypeData;
        using DiffType = Impl::Diff<TManager>;

        BitsetStorage bitsetIDs;
        ssvu::TplRepeat<TypeData, TManager::typeCount> typeDatas;

        inline static auto getJsonDiff(const ssvj::Val& mX, const ssvj::Val& mY)
        {
            auto result(ssvj::mkObj());

            auto xBitsStr(mX[jsonFieldFlagsKey].as<ssvj::Str>());
            // auto yBitsStr(mY[jsonFieldFlagsKey].as<ssvj::Str>());

            // ssvu::lo("XBITS") << xBitsStr << "\n";
            // ssvu::lo("YBITS") << yBitsStr << "\n";

            ObjBitset xFieldFlags{xBitsStr};
            // ObjBitset yFieldFlags{yBitsStr};

            auto toUp(xFieldFlags);

            for(auto i(0u); i < toUp.size(); ++i) {
                if(!toUp[i]) continue;

                std::string iStr{ssvu::toStr(i)};
                if(mX[iStr] == mY[iStr]) continue;

                result[iStr] = mX[iStr];
            }

            return result;
        }

        inline auto toJson() const
        {
            auto result(ssvj::mkArr(ssvj::mkArr(), // Bitset array
            ssvj::mkArr()                          // Data array
            ));

            for(const auto& b : bitsetIDs) result[0].emplace(b);
            ssvu::tplFor(
            [this, &result](const auto& mI)
            {
                result[1].emplace(mI.toJson());
            },
            typeDatas);
            return result;
        }

        inline void initFromJson(const ssvj::Val& mX)
        {
            for(auto i(0u); i < bitsetIDs.size(); ++i)
                bitsetIDs[i] = mX[0][i].as<ObjBitset>();
            ssvu::tplForData(
            [this, &mX](auto mD, auto& mTD)
            {
                mTD.initFromJson(mX[1][mD.getIdx()]);
            },
            typeDatas);
        }

        inline auto getDiffWith(const Snapshot& mX)
        {
            DiffType result;

            ssvu::tplForData(
            [this, &result, &mX](
            auto mD, auto& mTDCurrent, auto& mTDOther, auto& mTDDiff) mutable
            {
                // TODO: bitsetIDs need to be cleared somewhere
                // TODO: use ack/nack system?
                const auto& myBitset(bitsetIDs[mD.getIdx()]);
                const auto& diffBitset(mX.bitsetIDs[mD.getIdx()]);

                auto diffBitsetToCreate((~diffBitset) & myBitset);
                auto diffBitsetToRemove((~myBitset) & diffBitset);
                auto diffBitsetToUpdate(myBitset & diffBitset);

                for(auto i(0u); i < maxObjs; ++i) {
                    if(diffBitsetToCreate[i])
                        mTDDiff.toCreate[i] = mTDCurrent.items[i];
                    if(diffBitsetToRemove[i]) mTDDiff.toRemove.emplace_back(i);
                    if(diffBitsetToUpdate[i]) {
                        auto objDiff(Snapshot<TManager>::getJsonDiff(
                        mTDCurrent.items.at(i), mTDOther.items.at(i)));

                        // ssvu::lo("CURRN") << mTDCurrent.toJson() << "\n";
                        // ssvu::lo("OTHER") << mTDOther.toJson() << "\n";
                        // ssvu::lo() << "OBJDIFF" << objDiff << "\n\n";

                        if(!objDiff.isEmptyObj()) mTDDiff.toUpdate[i] = objDiff;
                    }
                }
            },
            typeDatas, mX.typeDatas, result.typeDatas);

            return result;
        }
    };
}
}

#endif
