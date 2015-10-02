#ifndef AUTOSYNCGEN_DIFFTYPEDATA
#define AUTOSYNCGEN_DIFFTYPEDATA

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"

namespace syn
{
namespace Impl
{
    /// @brief Class representing the diff for a specific type managed by the
    /// `SyncManager`.
    struct DiffTypeData
    {
        std::map<ID, ssvj::Val> toCreate, toUpdate;
        std::vector<ID> toRemove;

        /// @brief Returns a json value representing the diff type data.
        inline auto toJson() const
        {
            // TODO: better syntax in ssvj ?
            auto result(
            ssvj::mkArr(ssvj::mkObj(), ssvj::mkObj(), ssvj::mkObj()));

            auto& jCreate(result[jsonCreateIdx]);
            auto& jUpdate(result[jsonUpdateIdx]);
            auto& jRemove(result[jsonRemoveIdx]);

            for(const auto& x : toCreate)
                jCreate[ssvu::toStr(x.first)] = x.second;
            for(const auto& x : toUpdate)
                jUpdate[ssvu::toStr(x.first)] = x.second;
            for(const auto& x : toRemove) jRemove.emplace(x);

            return result;
        }

        /// @brief Initializes the diff type data from a json value `mX`.
        inline void initFromJson(const ssvj::Val& mX)
        {
            toCreate.clear();
            toRemove.clear();
            toUpdate.clear();

            const auto& jCreate(mX[jsonCreateIdx]);
            const auto& jUpdate(mX[jsonUpdateIdx]);
            const auto& jRemove(mX[jsonRemoveIdx]);

            for(const auto& x : jCreate.forObj())
                toCreate[ssvu::sToInt(x.key)] = x.value;
            for(const auto& x : jUpdate.forObj())
                toUpdate[ssvu::sToInt(x.key)] = x.value;
            for(const auto& x : jRemove.forArrAs<ID>())
                toRemove.emplace_back(x);
        }

        /// @brief Returns true if the diff type data is empty.
        inline bool isEmpty() const noexcept
        {
            return toCreate.empty() && toUpdate.empty() && toRemove.empty();
        }
    };
}
}

#endif
