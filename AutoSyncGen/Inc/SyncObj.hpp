#ifndef AUTOSYNCGEN_SYNCOBJ
#define AUTOSYNCGEN_SYNCOBJ

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/ObjBase.hpp"
#include "../../AutoSyncGen/Inc/FieldProxy.hpp"

namespace syn
{
	template<typename... TArgs> class SyncObj : public Impl::ObjBase
	{
		template<Idx, typename> friend class FieldProxy;
		template<typename> friend struct SerializationHelper;

		public:
			using TplFields = std::tuple<TArgs...>;

		private:
			static constexpr ssvu::SizeT fieldCount{sizeof...(TArgs)};
			TplFields fields;
			std::bitset<fieldCount> fieldFlags;

		public:
			template<Idx TI> using TypeAt = std::tuple_element_t<TI, decltype(fields)>;
			template<Idx TI> using ProxyAt = FieldProxy<TI, SyncObj<TArgs...>>;

		private:
			template<Idx TI> inline auto& getFieldAt() noexcept { return std::get<TI>(fields); }
			template<Idx TI> inline void setBitAt() noexcept { fieldFlags[TI] = true; }

		public:
			template<Idx TI> inline auto get() noexcept { return ProxyAt<TI>{*this}; }
			inline void resetFlags() noexcept { fieldFlags.reset(); }

			inline void setFromJson(const ssvj::Val& mX) { SerializationHelper<SyncObj>::setFromJson(mX, *this); }
			inline auto toJsonAll() { return SerializationHelper<SyncObj>::getAllToJson(*this); }
			inline auto toJsonChanged() { return SerializationHelper<SyncObj>::getChangedToJson(*this); }
	};
}

#endif