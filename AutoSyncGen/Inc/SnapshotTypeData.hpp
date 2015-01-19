#ifndef AUTOSYNCGEN_SNAPSHOTTYPEDATA
#define AUTOSYNCGEN_SNAPSHOTTYPEDATA

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"

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
	}
}

#endif