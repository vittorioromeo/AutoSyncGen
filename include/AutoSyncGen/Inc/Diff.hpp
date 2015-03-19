#ifndef AUTOSYNCGEN_DIFF
#define AUTOSYNCGEN_DIFF

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"

// TODO:
// 1) client sends SNAPSHOT
// 2) client receives DIFF
// 3) client applies DIFF to MANAGER

// ???
// 1) client gets initial server SNAPSHOT
// 2) client asks server for changes
// 3) server sends diff and applies the same changes to internally stored client snapshot (store it in connection data)
// 4) client asks server for changes... repeat

namespace syn
{
	namespace Impl
	{
		/// @brief Class representing a diff for all the types handled by the `SyncManager`.
		template<typename TManager> struct Diff
		{
			/// @brief Bitset of object flags.
			using ObjBitset = typename TManager::ObjBitset;

			/// @brief Type of the bitset storage.
			using BitsetStorage = typename TManager::BitsetStorage;

			/// @brief Type of the diff type data..
			using TypeData = DiffTypeData;

			/// @brief Tuple containing the diff type data for every type.
			ssvu::TplRepeat<TypeData, TManager::typeCount> typeDatas;

			/// @brief Returns a json value represeting the whole diff.
			inline auto toJson() const
			{
				auto result(ssvj::mkArr());
				ssvu::tplFor([this, &result](const auto& mI){ result.emplace(mI.toJson()); }, typeDatas);
				return result;
			}

			/// @brief Initializes the diff from a json value `mX`.
			inline void initFromJson(const ssvj::Val& mX)
			{
				ssvu::tplForIdx([this, &mX](auto mIdx, auto& mTD){ mTD.initFromJson(mX[mIdx]); }, typeDatas);
			}

			/// @brief Returns true if all diff type datas are empty.
			inline bool isEmpty() const noexcept
			{
				bool result{true};

				// TODO: is there a way of short-circuiting this?
				ssvu::tplFor([this, &result](const auto& mTD){ result &= mTD.isEmpty(); }, typeDatas);
				return result;
			}
		};
	}
}

#endif
