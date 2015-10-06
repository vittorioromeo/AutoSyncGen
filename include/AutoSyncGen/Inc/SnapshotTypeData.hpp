#ifndef AUTOSYNCGEN_SNAPSHOTTYPEDATA
#define AUTOSYNCGEN_SNAPSHOTTYPEDATA

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"

namespace syn
{
    namespace Impl
    {
        /// @brief Class representing the snapshot for a specific type managed
        /// by
        /// the `SyncManager`.
        struct SnapshotTypeData
        {
            /// @brief Items contained in the snapshot type data.
            std::map<ID, ssvj::Val> items;

            /// @brief Returns a json value representing the snapshot type data.
            inline auto toJson() const
            {
                auto result(ssvj::mkObj());

                for(const auto& x : items)
                    result[ssvu::toStr(x.first)] = x.second;
                return result;
            }

            /// @brief Initializes the snapshot type data from json.
            inline void initFromJson(const ssvj::Val& mX)
            {
                items.clear();

                for(const auto& x : mX.forObj())
                    items[ssvu::sToInt(x.key)] = x.value;
            }
        };
    }
}

#endif
