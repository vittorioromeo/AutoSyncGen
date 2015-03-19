#ifndef AUTOSYNCGEN_MANAGERHELPER
#define AUTOSYNCGEN_MANAGERHELPER

#include "../../AutoSyncGen/Inc/Common.hpp"

namespace syn
{
	namespace Impl
	{
		struct ManagerHelper
		{
			template<typename TManager, TypeIdx> inline static void initManager(TManager&) { }

			/// @brief Initializes the manager's runtime dispatch function arrays.
			template<typename TManager, TypeIdx TI, typename T, typename... TTypes>
			inline static void initManager(TManager& mX)
			{
				mX.funcsCreate[TI] = &TManager::template createImpl<T>;
				mX.funcsRemove[TI] = &TManager::template removeImpl<T>;
				mX.funcsUpdate[TI] = &TManager::template updateImpl<T>;

				initManager<TManager, TI + 1, TTypes...>(mX);
			}
		};
	}
}

#endif
