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
				using namespace ssvj;

				// TODO: better syntax in ssvj
				Val result{Arr{}};
				result.emplace(Obj{});
				result.emplace(Obj{});
				result.emplace(Obj{});

				// TODO: better syntax in ssvj
				auto& jCreate(result[jsonCreateIdx]);
				auto& jRemove(result[jsonRemoveIdx]);
				auto& jUpdate(result[jsonUpdateIdx]);

				for(const auto& x : toCreate) jCreate[ssvu::toStr(x.first)] = x.second;
				for(const auto& x : toRemove) jRemove.emplace(x);
				for(const auto& x : toUpdate) jUpdate[ssvu::toStr(x.first)] = x.second;

				return result;
			}

			inline void initFromJson(const ssvj::Val& mX)
			{
				const auto& jCreate(mX[jsonCreateIdx]);
				const auto& jRemove(mX[jsonRemoveIdx]);
				const auto& jUpdate(mX[jsonUpdateIdx]);

				for(const auto& x : jCreate.forObj()) toCreate[std::stoi(x.key)] = x.value;
				for(const auto& x : jRemove.forArrAs<ID>()) toRemove.emplace_back(x);
				for(const auto& x : jUpdate.forObj()) toUpdate[std::stoi(x.key)] = x.value;
			}
		};	
	}
}

#endif