#ifndef AUTOSYNCGEN_MANAGERHELPER
#define AUTOSYNCGEN_MANAGERHELPER

#include "../../AutoSyncGen/Inc/Common.hpp"

namespace syn
{
	namespace Impl
	{
		struct ManagerHelper
		{
			template<typename TManager, TypeIdx>
			inline static void initManager(TManager&)
			{

			}

			template<typename TManager, TypeIdx TI, typename T, typename... TTypes>
			inline static void initManager(TManager& mManager)
			{
				mManager.funcsCreate[TI] = &TManager::template createImpl<T>;
				mManager.funcsRemove[TI] = &TManager::template removeImpl<T>;
				mManager.funcsUpdate[TI] = &TManager::template updateImpl<T>;

				initManager<TManager, TI + 1, TTypes...>(mManager);
			}
		};
	}
}

#endif