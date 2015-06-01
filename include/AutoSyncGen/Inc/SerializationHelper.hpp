#ifndef AUTOSYNCGEN_SERIALIZATIONHELPER
#define AUTOSYNCGEN_SERIALIZATIONHELPER

#include "../../AutoSyncGen/Inc/Common.hpp"

namespace syn
{
	template<typename TObj> struct SerializationHelper
	{
		inline static void setFromJson(const ssvj::Val& mVal, TObj& mObj)
		{
			ssvu::tplForData([&mVal, &mObj](auto mD, auto& mField)
			{
				auto key(ssvu::toStr(mD.getIdx()));

				if(mVal.has(key))
				{
					mField = mVal[key].template as<ssvu::RmAll<decltype(mField)>>();
					mObj.unsetBitAt(mD.getIdx());
				}

			}, mObj.fields);
		}

		inline static auto getAllToJson(const TObj& mObj)
		{
			auto result(ssvj::mkObj
			(
				jsonFieldFlagsKey, mObj.fieldFlags
			));

			ssvu::tplForData([&result, &mObj](auto mD, auto&& mField)
			{
				auto key(ssvu::toStr(mD.getIdx()));
				result[key] = FWD(mField);

			}, mObj.fields);

			return result;
		}

		inline static auto getDirtyToJson(const TObj& mObj)
		{
			auto result(ssvj::mkObj
			(
				jsonFieldFlagsKey, mObj.fieldFlags
			));

			ssvu::tplForData([&result, &mObj](auto mD, auto&& mField)
			{
				if(mObj.fieldFlags[mD.getIdx()])
				{
					auto key(ssvu::toStr(mD.getIdx()));
					result[key] = FWD(mField);
				}

			}, mObj.fields);

			return result;
		}
	};
}

#endif
