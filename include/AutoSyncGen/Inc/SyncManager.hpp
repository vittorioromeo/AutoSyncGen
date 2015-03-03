#ifndef AUTOSYNCGEN_SYNCMANAGER
#define AUTOSYNCGEN_SYNCMANAGER

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"

namespace syn
{
	template<template<typename> class TLFManager, typename... TTypes> class SyncManager
	{
		friend struct syn::Impl::ManagerHelper;

		public:
			static constexpr SizeT typeCount{sizeof...(TTypes)};

			template<typename T> using LFManagerFor = TLFManager<T>;
			template<typename T> using HandleFor = typename LFManagerFor<T>::Handle;
			template<typename T> using HandleMapFor = std::map<ID, HandleFor<T>>;

			using ThisType = SyncManager<TLFManager, TTypes...>;
			using SnapshotType = Impl::Snapshot<ThisType>;
			using DiffType = Impl::Diff<ThisType>;
			using ObjBitset = std::bitset<maxObjs>;

			// TODO: tuple?
			using BitsetStorage = std::array<ObjBitset, typeCount>;

		private:

			using TplLFManagers = ssvu::Tpl<LFManagerFor<TTypes>...>;
			using TplHandleMaps = ssvu::Tpl<HandleMapFor<TTypes>...>;
			using TplIDs = ssvu::TplRepeat<ID, typeCount>;

			using MemFnCreate = void(SyncManager<TLFManager, TTypes...>::*)(ID, const ssvj::Val&);
			using MemFnRemove = void(SyncManager<TLFManager, TTypes...>::*)(ID);
			using MemFnUpdate = void(SyncManager<TLFManager, TTypes...>::*)(ID, const ssvj::Val&);

			TplLFManagers lfManagers;
			TplHandleMaps handleMaps;
			TplIDs lastIDs;

			std::array<MemFnCreate, typeCount> funcsCreate;
			std::array<MemFnRemove, typeCount> funcsRemove;
			std::array<MemFnUpdate, typeCount> funcsUpdate;

			BitsetStorage bitsetIDs;

			void testFn(int) { }

			template<typename T> inline auto& getBitsetFor() noexcept { return bitsetIDs[getTypeID<T>()]; }
			template<typename T> inline const auto& getBitsetFor() const noexcept { return bitsetIDs[getTypeID<T>()]; }
			template<typename T> inline bool isPresent(ID mID) const noexcept { return getBitsetFor<T>()[mID]; }
			template<typename T> inline void setPresent(ID mID, bool mX) noexcept { getBitsetFor<T>()[mID] = mX; }

			template<typename T> inline void createImpl(ID mID, const ssvj::Val& mVal)
			{
				SSVU_ASSERT(!isPresent<T>(mID)); // TODO: this fires after sending msg and receiving because the function is called twice ???

				setPresent<T>(mID, true);

				auto& handle(getHandleFor<T>(mID));
				handle = getLFManagerFor<T>().create();

				handle->setFromJson(mVal);

				getHandleMapFor<T>()[mID] = handle;
			}
			template<typename T> inline void removeImpl(ID mID)
			{
				SSVU_ASSERT(isPresent<T>(mID));
				setPresent<T>(mID, false);

				auto& handle(getHandleFor<T>(mID));
				getLFManagerFor<T>().remove(handle);
				handle = getNullHandleFor<T>();

				// TODO: null handle?
				getHandleMapFor<T>().erase(mID);
			}
			template<typename T> inline void updateImpl(ID mID, const ssvj::Val& mVal)
			{
				SSVU_ASSERT(isPresent<T>(mID));

				auto& handle(getHandleFor<T>(mID));
				handle->setFromJson(mVal);
			}

		public:
			inline SyncManager()
			{
				Impl::ManagerHelper::initManager<SyncManager<TLFManager, TTypes...>, 0, TTypes...>(*this);
			}

			template<typename T> inline static constexpr ID getTypeID() noexcept { return ssvu::getTplIdxOf<T, ssvu::Tpl<TTypes...>>(); }

			// TODO: ID pool, recycle deleted IDs
			template<typename T> inline ID getFirstFreeID() noexcept { return std::get<getTypeID<T>()>(lastIDs)++; }

			template<typename T> inline auto getNullHandleFor() noexcept { return getLFManagerFor<T>().getNullHandle(); }
			template<typename T> inline auto& getLFManagerFor() noexcept { return std::get<LFManagerFor<T>>(lfManagers); }
			template<typename T> inline auto& getHandleMapFor() noexcept { return std::get<HandleMapFor<T>>(handleMaps); }

			template<typename T> inline auto& getHandleFor(ID mID) noexcept { return getHandleMapFor<T>()[mID]; }

			// TODO: rename?
			/*template<typename T> inline auto& serverCreate2()
			{
				T data(ssvu::fwd<TArgs>(mArgs)...);
				return serverCreate<T>(data.toJsonAll());
			}*/
			template<typename T> inline auto serverCreate(const ssvj::Val& mVal)
			{
				auto createID(getFirstFreeID<T>());
				(this->*funcsCreate[getTypeID<T>()])(createID, mVal);
				return getHandleMapFor<T>()[createID];
			}

			inline auto getNullBitsetStorage()
			{
				return BitsetStorage{};
			}

			inline void onReceivedPacketCreate(ID mIDType, ID mID, const ssvj::Val& mVal)
			{
				(this->*funcsCreate[mIDType])(mID, mVal);
			}

			inline void onReceivedPacketRemove(ID mIDType, ID mID)
			{
				(this->*funcsRemove[mIDType])(mID);
			}

			inline void onReceivedPacketUpdate(ID mIDType, ID mID, const ssvj::Val& mVal)
			{
				(this->*funcsUpdate[mIDType])(mID, mVal);
			}

			inline void applyDiff(const DiffType& mX)
			{
				ssvu::tplForIdx([this, &mX](auto mIType, auto& mTD) mutable
				{
					//for(auto i(0u); i < maxObjs; ++i)
					{
						// TODO: can be compile-time?
						for(const auto& p : mTD.toCreate) this->onReceivedPacketCreate(mIType, p.first, p.second);
						for(auto x : mTD.toRemove) this->onReceivedPacketRemove(mIType, x);
						for(const auto& p : mTD.toUpdate) this->onReceivedPacketUpdate(mIType, p.first, p.second);
					}
				}, mX.typeDatas);
			}

			inline auto getSnapshot()
			{
				SnapshotType result;
				result.bitsetIDs = bitsetIDs;

				ssvu::tplFor([this, &result](auto& mHM, auto& mTD) mutable
				{
					for(const auto& x : mHM)
					{
						mTD.items[x.first] = x.second->toJsonAll();
					}
				}, handleMaps, result.typeDatas);

				return result;
			}


	};
}

#endif
