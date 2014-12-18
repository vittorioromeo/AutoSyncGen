#ifndef AUTOSYNCGEN_SERIALIZATIONHELPER
#define AUTOSYNCGEN_SERIALIZATIONHELPER

#include "../../AutoSyncGen/Inc/Common.hpp"

namespace syn
{
	template<typename TObj> struct SerializationHelper
	{
		inline static void setFromJson(const ssvj::Val& mVal, TObj& mObj)
		{
			ssvu::tplForIdx([&mVal, &mObj](auto mIdx, auto& mField)
			{
				auto key(ssvu::toStr(mIdx));
				
				if(mVal.has(key))
				{
					mField = mVal[key].template as<ssvu::RemoveAll<decltype(mField)>>();
				}
				
			}, mObj.fields);
		}

		inline static auto getAllToJson(const TObj& mObj)
		{
			ssvj::Val result{ssvj::Obj{}};

			ssvu::tplForIdx([&result, &mObj](auto mIdx, auto&& mField)
			{
				auto key(ssvu::toStr(mIdx));
				result[key] = ssvu::fwd<decltype(mField)>(mField);

			}, mObj.fields);

			return result;
		}

		inline static auto getChangedToJson(const TObj& mObj)
		{
			ssvj::Val result{ssvj::Obj{}};

			ssvu::tplForIdx([&result, &mObj](auto mIdx, auto&& mField)
			{
				if(mObj.fieldFlags[mIdx])
				{ 
					auto key(ssvu::toStr(mIdx));
					result[key] = ssvu::fwd<decltype(mField)>(mField);
				}

			}, mObj.fields);

			return result;
		}
	};
}

#endif