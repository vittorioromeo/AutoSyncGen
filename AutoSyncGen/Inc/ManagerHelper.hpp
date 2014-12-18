#ifndef AUTOSYNCGEN_MANAGERHELPER
#define AUTOSYNCGEN_MANAGERHELPER

#include "../../AutoSyncGen/Inc/Common.hpp"

namespace syn
{
	struct ManagerHelper
	{
		template<typename TManager, ssvu::SizeT>
		inline static void initManager(TManager&)
		{

		}

		template<typename TManager, ssvu::SizeT TI, typename T, typename... TTypes>
		inline static void initManager(TManager& mManager)
		{
			mManager.funcsCreate[TI] = &TManager::template createImpl<T>;
			mManager.funcsRemove[TI] = &TManager::template removeImpl<T>;
			mManager.funcsUpdate[TI] = &TManager::template updateImpl<T>;

			initManager<TManager, TI + 1, TTypes...>(mManager);
		}
	};
}

#endif