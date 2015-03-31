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
				auto key(ssvu::toStr(ssvu::getIdx(mD)));

				if(mVal.has(key))
				{
					mField = mVal[key].template as<ssvu::RmAll<decltype(mField)>>();
					mObj.unsetBitAt(ssvu::getIdx(mD));
				}

			}, mObj.fields);
		}

		inline static auto getAllToJson(const TObj& mObj)
		{
			auto result(ssvj::mkObj());

			// TODO: serialize bitset
			result[jsonFieldFlagsKey] = mObj.fieldFlags.to_string();

			ssvu::tplForData([&result, &mObj](auto mD, auto&& mField)
			{
				auto key(ssvu::toStr(ssvu::getIdx(mD)));
				result[key] = FWD(mField);

			}, mObj.fields);

			return result;
		}

		inline static auto getDirtyToJson(const TObj& mObj)
		{
			auto result(ssvj::mkObj());

			// TODO: serialize bitset, code repetition
			result[jsonFieldFlagsKey] = mObj.fieldFlags.to_string();

			ssvu::tplForData([&result, &mObj](auto mD, auto&& mField)
			{
				if(mObj.fieldFlags[ssvu::getIdx(mD)])
				{
					auto key(ssvu::toStr(ssvu::getIdx(mD)));
					result[key] = FWD(mField);
				}

			}, mObj.fields);

			return result;
		}
	};
}

#endif
