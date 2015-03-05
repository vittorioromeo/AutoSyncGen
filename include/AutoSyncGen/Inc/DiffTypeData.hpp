#ifndef AUTOSYNCGEN_DIFFTYPEDATA
#define AUTOSYNCGEN_DIFFTYPEDATA

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"

namespace syn
{
	namespace Impl
	{
		struct DiffTypeData
		{
			std::map<ID, ssvj::Val> toCreate, toUpdate;
			std::vector<ID> toRemove;

			inline auto toJson() const
			{
				// TODO: better syntax in ssvj ?
				auto result(ssvj::mkArr(ssvj::mkObj(), ssvj::mkObj(), ssvj::mkObj()));

				auto& jCreate(result[jsonCreateIdx]);
				auto& jUpdate(result[jsonUpdateIdx]);
				auto& jRemove(result[jsonRemoveIdx]);

				for(const auto& x : toCreate) jCreate[ssvu::toStr(x.first)] = x.second;
				for(const auto& x : toUpdate) jUpdate[ssvu::toStr(x.first)] = x.second;
				for(const auto& x : toRemove) jRemove.emplace(x);

				return result;
			}

			inline void initFromJson(const ssvj::Val& mX)
			{
				toCreate.clear();
				toRemove.clear();
				toUpdate.clear();

				const auto& jCreate(mX[jsonCreateIdx]);
				const auto& jUpdate(mX[jsonUpdateIdx]);
				const auto& jRemove(mX[jsonRemoveIdx]);

				for(const auto& x : jCreate.forObj()) toCreate[std::stoi(x.key)] = x.value;
				for(const auto& x : jUpdate.forObj()) toUpdate[std::stoi(x.key)] = x.value;
				for(const auto& x : jRemove.forArrAs<ID>()) toRemove.emplace_back(x);
			}
		};
	}
}

#endif
