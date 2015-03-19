#ifndef AUTOSYNCGEN_SYNCOBJ
#define AUTOSYNCGEN_SYNCOBJ

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/ObjBase.hpp"
#include "../../AutoSyncGen/Inc/FieldProxy.hpp"

namespace syn
{
	template<typename... TArgs> class SyncObj : public Impl::ObjBase
	{
		template<TypeIdx, typename> friend class FieldProxy;
		template<typename> friend struct SerializationHelper;

		public:
			/// @brief Type of the tuple of fields.
			using TplFields = ssvu::Tpl<TArgs...>;

		private:
			/// @brief Count of fields.
			static constexpr SizeT fieldCount{sizeof...(TArgs)};

			/// @brief Tuple of fields.
			TplFields fields;

			/// @brief "Dirty" bits for the fields.
			std::bitset<fieldCount> fieldFlags;

		public:
			/// @brief Type of the field at index `TI`.
			template<TypeIdx TI> using TypeAt = ssvu::TplElem<TI, decltype(fields)>;

			/// @brief Type of the field proxy at index `TI`.
			template<TypeIdx TI> using ProxyAt = FieldProxy<TI, SyncObj<TArgs...>>;

		private:
			/// @brief Non-const reference to the field at index `TI`.
			template<TypeIdx TI> inline auto& getFieldAt() noexcept
			{
				return std::get<TI>(fields);
			}

			/// @brief Sets the bit at index `TI` to true.
			template<TypeIdx TI> inline void setBitAt() noexcept
			{
				fieldFlags[TI] = true;
			}

			/// @brief Sets the bit at index `mI` to false.
			inline void unsetBitAt(TypeIdx mI) noexcept
			{
				fieldFlags[mI] = false;
			}

		public:
			/// @brief Returns a proxy for the field at index `TI`.
			template<TypeIdx TI> inline auto get() noexcept
			{
				return ProxyAt<TI>{*this};
			}

			/// @brief Sets the object state from an `mX` json value.
			/// @details The `SerializationHelper` object will reset field bits.
			inline void setFromJson(const ssvj::Val& mX)
			{
				SerializationHelper<SyncObj>::setFromJson(mX, *this);
			}

			/// @brief Returns a json value of all the fields.
			inline auto toJsonAll()
			{
				return SerializationHelper<SyncObj>::getAllToJson(*this);
			}

			/// @brief Returns a json value of all the dirty fields.
			inline auto toJsonDirty()
			{
				return SerializationHelper<SyncObj>::getDirtyToJson(*this);
			}
	};
}

#endif
