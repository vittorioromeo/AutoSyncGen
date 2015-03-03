#ifndef AUTOSYNCGEN_SNAPSHOTTYPEDATA
#define AUTOSYNCGEN_SNAPSHOTTYPEDATA

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"

namespace syn
{
	namespace Impl
	{
		struct SnapshotTypeData
		{
			std::map<ID, ssvj::Val> items;

			inline auto toJson() const
			{
				auto result(ssvj::mkObj());
				for(const auto& x : items) result[ssvu::toStr(x.first)] = x.second;
				return result;
			}

			inline void initFromJson(const ssvj::Val& mX)
			{
				items.clear();
				for(const auto& x : mX.forObj()) items[std::stoi(x.key)] = x.value;
			}
		};
	}
}

#endif
