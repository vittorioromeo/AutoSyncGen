#ifndef AUTOSYNCGEN_SERIALIZATIONHELPER
#define AUTOSYNCGEN_SERIALIZATIONHELPER

#include "../../AutoSyncGen/Inc/Common.hpp"

namespace syn
{
	template<typename TObj> struct SerializationHelper
	{
		inline static void setFromJson(const ssvj::Val& mVal, TObj& mObj)
		{
			// TODO: set dirty bits or not?

			ssvu::tplForIdx([&mVal, &mObj](auto mIdx, auto& mField)
			{
				auto key(ssvu::toStr(mIdx));

				if(mVal.has(key))
				{
					mField = mVal[key].template as<ssvu::RmAll<decltype(mField)>>();
				}

			}, mObj.fields);
		}

		inline static auto getAllToJson(const TObj& mObj)
		{
			auto result(ssvj::mkObj());

			// TODO: serialize bitset
			result[jsonFieldFlagsKey] = mObj.fieldFlags.to_string();

			ssvu::tplForIdx([&result, &mObj](auto mIdx, auto&& mField)
			{
				auto key(ssvu::toStr(mIdx));
				result[key] = FWD(mField);

			}, mObj.fields);

			return result;
		}

		inline static auto getChangedToJson(const TObj& mObj)
		{
			auto result(ssvj::mkObj());

			// TODO: serialize bitset, code repetition
			result[jsonFieldFlagsKey] = mObj.fieldFlags.to_string();

			ssvu::tplForIdx([&result, &mObj](auto mIdx, auto&& mField)
			{
				if(mObj.fieldFlags[mIdx])
				{
					auto key(ssvu::toStr(mIdx));
					result[key] = FWD(mField);
				}

			}, mObj.fields);

			return result;
		}
	};
}

#endif
